#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <libblossom/blossom.h>

#if defined(__linux__)
#include <sched.h>
#elif defined(__freebsd__)
#include <sys/param.h>
#include <sys/cpuset.h>
typedef cpuset_t cpu_set_t;
#else
#error "Need cpu_set_t definition for this platform" // solaris is psetid_t
#endif

// FreeBSD's cpuset.h (as of 7.2) doesn't provide CPU_COUNT, nor do older
// Linux setups (including RHEL5). This requires only CPU_{SETSIZE,ISSET}.
static inline unsigned
portable_cpuset_count(void){
	unsigned count = 0,cpu;
	cpu_set_t mask;

#if defined(__freebsd__)
	if(cpuset_getaffinity(CPU_LEVEL_CPUSET,CPU_WHICH_CPUSET,-1,
				sizeof(mask),&mask) == 0){
#elif defined(__linux__)
	// We might be only a subthread of a larger application; use the
	// affinity mask of the thread which initializes us.
	if(pthread_getaffinity_np(pthread_self(),sizeof(mask),&mask) == 0){
#else
#error "Need cpu_set_t definition for this platform" // solaris is psetid_t
#endif
		for(cpu = 0 ; cpu < CPU_SETSIZE ; ++cpu){
			if(CPU_ISSET(cpu,&mask)){
				++count;
			}
		}
	}
	return count;
}

// This is the simple, O(N) latency pthread creation
/* int blossom_pthreads(blossom_state *ctx,const pthread_attr_t *attr,
			void *(*fxn)(void *),void *arg){
	unsigned z;

	for(z = 0 ; z < ctx->tidcount ; ++z){
		int ret;

		if( (ret = pthread_create(&ctx->tids[z],attr,fxn,arg)) ){
			// FIXME cleanup spawned threads...how?
			return ret;
		}
	}
	return 0;
} */

enum {
	BLOOM_INITIALIZED,
	BLOOM_SUCCESS,
	BLOOM_EXCEPTION,
};

typedef struct blossom {
	unsigned idx;
	blossom_state *ctx;
	int result;
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	const pthread_attr_t *attr;
	void *(*fxn)(void *);
	void *arg;
} blossom;

static inline pthread_t *
create_tid_state(unsigned z){
	return malloc(sizeof(pthread_t) * z);
}

static blossom *
create_blossom(unsigned idx,blossom_state *ctx,const pthread_attr_t *attr,
			void *(*fxn)(void *),void *arg){
	blossom *b;

	if((b = malloc(sizeof(*b))) ){
		if(pthread_cond_init(&b->cond,NULL)){
			free(b);
			return NULL;
		}
		if(pthread_mutex_init(&b->mutex,NULL)){
			pthread_cond_destroy(&b->cond);
			free(b);
			return NULL;
		}
		b->idx = idx;
		b->ctx = ctx;
		b->result = BLOOM_INITIALIZED;
		b->attr = attr;
		b->fxn = fxn;
		b->arg = arg;
	}
	return b;
}

static int
free_blossom(blossom *b){
	int ret = 0;

	if(b){
		ret |= pthread_cond_destroy(&b->cond);
		ret |= pthread_mutex_destroy(&b->mutex);
		free(b);
	}
	return ret;
}

static int
block_on_bloom(blossom *b){
	int ret,result;

	if( (ret = pthread_mutex_lock(&b->mutex)) ){
		return ret;
	}
	while(b->result == BLOOM_INITIALIZED){
		if( (ret = pthread_cond_wait(&b->cond,&b->mutex)) ){
			return ret;
		}
	}
	result = b->result;
	if( (ret = pthread_mutex_unlock(&b->mutex)) ){
		return ret;
	}
	if(result == BLOOM_SUCCESS){
		return 0;
	}
	return EAGAIN; // FIXME pthread_create() resource error. others?
}

static inline unsigned
idx_left(unsigned idx){
	return 2 * idx + 1;
}

static inline unsigned
idx_right(unsigned idx){
	return 2 * idx + 2;
}

#define RET_BLOOM_FAILURE do{ \
		pthread_mutex_lock(&b->mutex); \
		b->result = BLOOM_EXCEPTION; \
		pthread_cond_signal(&b->cond); \
		pthread_mutex_unlock(&b->mutex); \
		return NULL; \
	}while(0)

static int
mark_bloom_success(blossom *b){
	if(pthread_mutex_lock(&b->mutex)){
		return -1;
	}
	b->result = BLOOM_SUCCESS;
	pthread_cond_signal(&b->cond);
	pthread_mutex_unlock(&b->mutex);
	return 0;
}

static void *
blossom_thread(void *unsafeb){
	blossom *b = unsafeb;
	void *(*fxn)(void *);
	void *arg;

	if(idx_left(b->idx) < b->ctx->tidcount){
		blossom *bl;

		if((bl = create_blossom(idx_left(b->idx),b->ctx,b->attr,
					b->fxn,b->arg)) == NULL){
			RET_BLOOM_FAILURE;
		}
		if(pthread_create(&bl->ctx->tids[idx_left(b->idx)],
					bl->attr,blossom_thread,bl)){
			free_blossom(bl);
			RET_BLOOM_FAILURE;
		}
		if(idx_right(b->idx) < b->ctx->tidcount){
			blossom *br;

			if((br = create_blossom(idx_right(b->idx),b->ctx,
					b->attr,b->fxn,b->arg)) == NULL){
				RET_BLOOM_FAILURE;
			}
			if(pthread_create(&br->ctx->tids[idx_right(b->idx)],
						br->attr,blossom_thread,br)){
				// FIXME reap left
				RET_BLOOM_FAILURE;
			}
			if(block_on_bloom(br)){
				// FIXME reap both
				RET_BLOOM_FAILURE;
			}
			free_blossom(br);
		}
		if(block_on_bloom(bl)){
			// FIXME reap
			free_blossom(bl);
			RET_BLOOM_FAILURE;
		}
		free_blossom(bl);
	}
	fxn = b->fxn;
	arg = b->arg;
	if(mark_bloom_success(b)){
		RET_BLOOM_FAILURE;
	}
	return fxn(arg);
}

void blossom_free_state(blossom_state *ctx){
	if(ctx){
		free(ctx->tids);
		ctx->tids = NULL;
		ctx->tidcount = 0;
	}
}

static int
blossom_1_thread(unsigned idx,blossom_state *ctx,const pthread_attr_t *attr,
			void *(*fxn)(void *),void *arg,
			void *(*blfxn)(void *)){
	blossom *b;
	int ret;

	if((b = create_blossom(idx,ctx,attr,fxn,arg)) == NULL){
		return errno;
	}
	if( (ret = pthread_create(&ctx->tids[idx],attr,blfxn,b)) ){
		free_blossom(b);
		return ret;
	}
	if( (ret = block_on_bloom(b)) ){
		// FIXME reap
		free_blossom(b);
		return ret;
	}
	if( (ret = free_blossom(b)) ){
		return ret;
	}
	return 0;
}

// Blossom precisely as many threads as are specified by tidcount.
static int
blossom_n_threads(blossom_state *ctx,const pthread_attr_t *attr,
			void *(*fxn)(void *),void *arg,
			void *(*blfxn)(void *)){
	int ret;

	if(ctx->tidcount <= 0){
		return EINVAL;
	}
	if((ctx->tids = create_tid_state(ctx->tidcount)) == NULL){
		return errno;
	}
	if( (ret = blossom_1_thread(0,ctx,attr,fxn,arg,blfxn)) ){
		blossom_free_state(ctx);
		return ret;
	}
	return 0;
}

// pthreads return semantics (positive error code in result, errno ignored)
int blossom_pthreads(unsigned tids,blossom_state *ctx,const pthread_attr_t *attr,
			void *(*fxn)(void *),void *arg){
	int ret;

	ctx->tidcount = tids;
	if( (ret = blossom_n_threads(ctx,attr,fxn,arg,blossom_thread)) ){
		return ret;
	}
	return 0;
}

int blossom_per_pe(unsigned tids,blossom_state *ctx,const pthread_attr_t *attr,
			void *(*fxn)(void *),void *arg){
	unsigned cpucount;
	int ret;

	if((cpucount = portable_cpuset_count()) == 0){
		return errno;
	}
	ctx->tidcount = tids * cpucount; // FIXME check for overflow
	if( (ret = blossom_n_threads(ctx,attr,fxn,arg,blossom_thread)) ){
		blossom_free_state(ctx);
		return ret;
	}
	return 0;
}

struct pestruct {
	unsigned dcpu;
	void *arg;
};

static void *
blossom_pe_thread(void *unsafeb){
	struct pestruct *pe;
	blossom *b = unsafeb;
	void *(*fxn)(void *);
	cpu_set_t mask;
	void *arg;
	int dcpu;

	pe = b->arg;
	dcpu = b->result;
	CPU_ZERO(&mask);
	CPU_SET(pe->dcpu,&mask);
#if defined(__freebsd__)
	if(cpuset_setaffinity(CPU_LEVEL_WHICH,CPU_WHICH_TID,-1,
				sizeof(mask),&mask)){
#else
	if(sched_setaffinity(0,sizeof(mask),&mask)){
#endif
		RET_BLOOM_FAILURE;
	}
	fxn = b->fxn;
	arg = pe->arg;
	if(mark_bloom_success(b)){
		RET_BLOOM_FAILURE;
	}
	return fxn(arg);
}

int blossom_on_pe(unsigned tids,blossom_state *ctx,const pthread_attr_t *attr,
			void *(*fxn)(void *),void *arg){
	cpu_set_t mask;
	unsigned cpu;
	int ret;

	ctx->tidcount = 0;
	ctx->tids = NULL;
#if defined(__freebsd__)
	if(cpuset_getaffinity(CPU_LEVEL_CPUSET,CPU_WHICH_CPUSET,-1,
				sizeof(mask),&mask) == 0){
#elif defined(__linux__)
	// We might be only a subthread of a larger application; use the
	// affinity mask of the thread which initializes us.
	if(pthread_getaffinity_np(pthread_self(),sizeof(mask),&mask) == 0){
#else
#error "Need cpu_set_t definition for this platform" // solaris is psetid_t
#endif
		for(cpu = 0 ; cpu < CPU_SETSIZE ; ++cpu){
			if(CPU_ISSET(cpu,&mask)){
				typeof(*ctx->tids) *tmp;
				struct pestruct curry = {
					.dcpu = cpu,
					.arg = arg,
				};
				unsigned z;

				if((tmp = realloc(ctx->tids,(ctx->tidcount + tids) * sizeof(*tmp))) == NULL){
					// FIXME reap backward
					blossom_free_state(ctx);
					return errno;
				}
				ctx->tids = tmp;
				for(z = 0 ; z < tids ; ++z){
					if( (ret = blossom_1_thread(ctx->tidcount,ctx,attr,fxn,
								&curry,blossom_pe_thread)) ){
						// FIXME reap backward
						return ret;
					}
					++ctx->tidcount;
				}
			}
		}
		return 0;
	}
	return errno;
}

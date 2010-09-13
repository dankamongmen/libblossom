#include <errno.h>
#include <stdlib.h>
#include <libblossom/blossom.h>
#include <libblossom/schedule.h>

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
	while(b->result == 0){
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
	return -1;
}

static inline unsigned
idx_left(unsigned idx){
	return 2 * idx + 1;
}

static inline unsigned
idx_right(unsigned idx){
	return 2 * idx + 2;
}

#define BLOOM_FAILURE do{ \
		pthread_mutex_lock(&b->mutex); \
		b->result = BLOOM_EXCEPTION; \
		pthread_mutex_unlock(&b->mutex); \
		return NULL; \
	}while(0)

static void *
blossom_thread(void *unsafeb){
	blossom *b = unsafeb;

	if(idx_left(b->idx) < b->ctx->tidcount){
		blossom *bl;

		if((bl = create_blossom(idx_left(b->idx),b->ctx,b->attr,
					b->fxn,b->arg)) == NULL){
			BLOOM_FAILURE;
		}
		if(pthread_create(&b->ctx->tids[idx_left(b->idx)],
					b->attr,blossom_thread,bl)){
			free_blossom(bl);
			BLOOM_FAILURE;
		}
		if(idx_right(b->idx) < b->ctx->tidcount){
			blossom *br;

			if((br = create_blossom(idx_right(b->idx),b->ctx,
					b->attr,b->fxn,b->arg)) == NULL){
				BLOOM_FAILURE;
			}
			if(pthread_create(&b->ctx->tids[idx_right(b->idx)],
						b->attr,blossom_thread,br)){
				// FIXME reap left
				BLOOM_FAILURE;
			}
			if(block_on_bloom(br)){
				// FIXME reap both
				BLOOM_FAILURE;
			}
			free_blossom(br);
		}
		if(block_on_bloom(bl)){
			// FIXME reap
			free_blossom(bl);
			BLOOM_FAILURE;
		}
		free_blossom(bl);
	}
	pthread_mutex_lock(&b->mutex);
	b->result = BLOOM_SUCCESS;
	pthread_cond_signal(&b->cond);
	pthread_mutex_unlock(&b->mutex);
	pthread_exit(b->fxn(b->arg));
}

void blossom_free_state(blossom_state *ctx){
	if(ctx){
		free(ctx->tids);
	}
}

// pthreads return semantics (positive error code in result, errno ignored)
int blossom_pthreads(blossom_state *ctx,const pthread_attr_t *attr,
			void *(*fxn)(void *),void *arg){
	blossom *b;
	int ret;

	if(ctx->tidcount <= 0){
		return EINVAL;
	}
	if((ctx->tids = create_tid_state(ctx->tidcount)) == NULL){
		return errno;
	}
	if((b = create_blossom(0,ctx,attr,fxn,arg)) == NULL){
		blossom_free_state(ctx);
		return errno;
	}
	if( (ret = pthread_create(&ctx->tids[0],attr,blossom_thread,b)) ){
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

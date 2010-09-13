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

typedef struct blossom {
	unsigned idx;
	blossom_state *ctx;
	const pthread_attr_t *attr;
	void *(*fxn)(void *);
	void *arg;
} blossom;

static inline pthread_t *
create_tid_state(unsigned z){
	return malloc(sizeof(pthread_t) * z);
}

static void *
blossom_thread(void *unsafeb){
	blossom *b = unsafeb;

	if(b->idx != b->ctx->tidcount){
		// spawn new one FIXME
	}
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
	if((b = malloc(sizeof(*b))) == NULL){
		blossom_free_state(ctx);
		return errno;
	}
	b->idx = 0;
	b->ctx = ctx;
	b->attr = attr;
	b->fxn = fxn;
	b->arg = arg;
	if( (ret = pthread_create(&ctx->tids[0],attr,blossom_thread,b)) ){
		free(b);
		return ret;
	}
	// FIXME verify all were spawned
	free(b);
	return 0;
}

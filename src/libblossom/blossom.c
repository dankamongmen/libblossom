#include <errno.h>
#include <stdlib.h>
#include <libblossom/blossom.h>

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
	void *arg;
} blossom;

static void *
blossom_thread(void *unsafeb){
	blossom *b = unsafeb;

	if(b->idx != b->ctx->tidcount){
		// spawn new one FIXME
	}
	pthread_exit(NULL);
}

// pthreads return semantics (positive error code in result, errno ignored)
int blossom_pthreads(blossom_state *ctx,const pthread_attr_t *attr,
			void *(*fxn)(void *),void *arg){
	unsigned z;
	blossom *b;
	int ret;

	if(ctx->tidcount <= 0){
		return EINVAL;
	}
	if((b = malloc(sizeof(*b))) == NULL){
		return errno;
	}
	b->idx = 0;
	b->ctx = ctx;
	b->attr = attr;
	b->arg = arg;
	if( (ret = pthread_create(&ctx->tids[0],attr,fxn,b)) ){
		free(b);
		return ret;
	}
	// FIXME verify all were spawned
	return 0;
}

#include <libblossom/blossom.h>

int blossom_pthreads(blossom_state *ctx,const pthread_attr_t *attr,
			void *(*fxn)(void *),void *arg){
	return pthread_create(ctx->tids,attr,fxn,arg);
}

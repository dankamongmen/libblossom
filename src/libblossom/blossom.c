#include <libblossom/blossom.h>

int blossom_pthreads(blossom_state *ctx,const pthread_attr_t *attr,
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
}

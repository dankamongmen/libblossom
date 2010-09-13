#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <libblossom/blossom.h>

static blossom_state bloom = {
	.tids = NULL,
	.tidcount = 1,
};

static blossom_state bloom128 = {
	.tids = NULL,
	.tidcount = 128,
};

static void *
argfxn(void *v){
	printf("Argument: %p\n",v);
	pthread_exit(v);
}

static int
do_bloom(int (*fxn)(blossom_state *,const pthread_attr_t *,void *(*)(void *),void *),blossom_state *ctx){
	void *arg = ctx;
	int ret,z;

	if( (ret = fxn(ctx,NULL,argfxn,arg)) ){
		fprintf(stderr,"blossom_pthreads returned %d (%s)\n",
				ret,strerror(ret));
		return -1;
	}
	for(z = 0 ; z < ctx->tidcount ; ++z){
		void *rarg;

		if( (ret = pthread_join(ctx->tids[z],&rarg)) ){
			fprintf(stderr,"pthread_join returned %d (%s)\n",
					ret,strerror(ret));
			return -1;
		}
		if(arg != rarg){
			fprintf(stderr,"pthread_join provided value %p, wanted %p\n",rarg,arg);
			return -1;
		}
		printf("Joined (Verified argument (%p))\n",arg);
	}
	printf("Reaped %u threads.\n",ctx->tidcount);
	blossom_free_state(ctx);
	return 0;
}

int main(void){
	printf("Testing libblossom...\n");
	if(do_bloom(blossom_pthreads,&bloom)){
		return EXIT_FAILURE;
	}
	if(do_bloom(blossom_per_pe,&bloom)){
		return EXIT_FAILURE;
	}
	if(do_bloom(blossom_pthreads,&bloom128)){
		return EXIT_FAILURE;
	}
	if(do_bloom(blossom_per_pe,&bloom128)){
		return EXIT_FAILURE;
	}
	printf("Tests succeeded.\n");
	return EXIT_SUCCESS;
}

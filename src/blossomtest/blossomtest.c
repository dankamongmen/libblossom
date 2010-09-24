#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <blossom.h>

static void *
argfxn(void *v){
	printf("Argument: %p\n",v);
	pthread_exit(v);
}

static int
do_bloom(const blossom_ctl *ctl,int (*fxn)(const blossom_ctl *,blossom_state *,const pthread_attr_t *,void *(*)(void *),void *),blossom_state *ctx){
	void *arg = ctx;
	unsigned z;
	int ret;

	if( (ret = fxn(ctl,ctx,NULL,argfxn,arg)) ){
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
		printf("Joined %u (Verified argument (%p))\n",z,arg);
	}
	printf("Reaped %u thread%s.\n",ctx->tidcount,ctx->tidcount == 1 ? "" : "s");
	blossom_free_state(ctx);
	return 0;
}

int main(void){
	blossom_state bloom;
	blossom_ctl ctl;

	printf("Testing libblossom...\n");
	ctl.flags = BLOSSOM_RUN_ON_FAILURE;
	ctl.tids = 1;
	if(do_bloom(&ctl,blossom_pthreads,&bloom)){
		return EXIT_FAILURE;
	}
	if(do_bloom(&ctl,blossom_per_pe,&bloom)){
		return EXIT_FAILURE;
	}
	if(do_bloom(&ctl,blossom_on_pe,&bloom)){
		return EXIT_FAILURE;
	}
	ctl.tids = 4;
	if(do_bloom(&ctl,blossom_pthreads,&bloom)){
		return EXIT_FAILURE;
	}
	if(do_bloom(&ctl,blossom_per_pe,&bloom)){
		return EXIT_FAILURE;
	}
	if(do_bloom(&ctl,blossom_on_pe,&bloom)){
		return EXIT_FAILURE;
	}
	printf("Tests succeeded.\n");
	return EXIT_SUCCESS;
}

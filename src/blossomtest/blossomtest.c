#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <libblossom/blossom.h>

static void *
argfxn(void *v){
	int cpu;

	cpu = sched_getcpu();
	printf("CPU: %u Argument: %p\n",cpu,v);
	pthread_exit(v);
}

static int
do_bloom(unsigned count,int (*fxn)(unsigned,blossom_state *,const pthread_attr_t *,void *(*)(void *),void *),blossom_state *ctx){
	void *arg = ctx;
	unsigned z;
	int ret;

	if( (ret = fxn(count,ctx,NULL,argfxn,arg)) ){
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

	printf("Testing libblossom...\n");
	if(do_bloom(1,blossom_pthreads,&bloom)){
		return EXIT_FAILURE;
	}
	if(do_bloom(1,blossom_per_pe,&bloom)){
		return EXIT_FAILURE;
	}
	if(do_bloom(1,blossom_on_pe,&bloom)){
		return EXIT_FAILURE;
	}
	/*
	if(do_bloom(128,blossom_pthreads,&bloom)){
		return EXIT_FAILURE;
	}
	if(do_bloom(128,blossom_per_pe,&bloom)){
		return EXIT_FAILURE;
	}
	if(do_bloom(128,blossom_on_pe,&bloom)){
		return EXIT_FAILURE;
	}
	*/
	printf("Tests succeeded.\n");
	return EXIT_SUCCESS;
}

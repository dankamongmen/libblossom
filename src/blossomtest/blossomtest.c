#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <libblossom/blossom.h>

static blossom_state bloom = {
	.tids = NULL,
	.tidcount = 1,
};

static void *
fxn(void *v){
	printf("Argument: %p\n",v);
	pthread_exit(v);
}

static int
do_bloom(blossom_state *ctx){
	int ret,z;

	if( (ret = blossom_pthreads(&bloom,NULL,fxn,NULL)) ){
		fprintf(stderr,"blossom_pthreads returned %d (%s)\n",
				ret,strerror(ret));
		return -1;
	}
	for(z = 0 ; z < bloom.tidcount ; ++z){
		void *arg;

		if( (ret = pthread_join(bloom.tids[z],&arg)) ){
			fprintf(stderr,"pthread_join returned %d (%s)\n",
					ret,strerror(ret));
			return -1;
		}
		if(arg){
			fprintf(stderr,"pthread_join provided value %p\n",arg);
			return -1;
		}
		printf("Joined (Verified argument (%p))\n",arg);
	}
	blossom_free_state(&bloom);
	return 0;
}

int main(void){
	printf("Testing libblossom...\n");
	if(do_bloom(&bloom)){
		return EXIT_FAILURE;
	}
	printf("Tests succeeded.\n");
	return EXIT_SUCCESS;
}

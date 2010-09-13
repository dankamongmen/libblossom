#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <libblossom/blossom.h>

static blossom_state bloom = {
	.tids = NULL,
	.tidcount = sizeof(bloom.tids) / sizeof(*bloom.tids),
};

static void *
fxn(void *v){
	printf("Argument: %p\n",v);
	pthread_exit(v);
}

int main(void){
	int ret,z;

	printf("Testing libblossom...\n");
	if( (ret = blossom_pthreads(&bloom,NULL,fxn,NULL)) ){
		fprintf(stderr,"blossom_pthreads returned %d (%s)\n",
				ret,strerror(ret));
		return EXIT_FAILURE;
	}
	for(z = 0 ; z < bloom.tidcount ; ++z){
		void *arg;

		if( (ret = pthread_join(bloom.tids[z],&arg)) ){
			fprintf(stderr,"pthread_join returned %d (%s)\n",
					ret,strerror(ret));
			return EXIT_FAILURE;
		}
		printf("Joined (Argument: %p)\n",arg);
	}
	blossom_free_state(&bloom);
	printf("Tests succeeded.\n");
	return EXIT_SUCCESS;
}

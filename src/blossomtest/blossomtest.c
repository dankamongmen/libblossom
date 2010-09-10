#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <libblossom/blossom.h>

static pthread_t tids[1];
static int tidcount = sizeof(tids) / sizeof(*tids);

static void *
fxn(void *v){
	printf("Argument: %p\n",v);
	pthread_exit(v);
}

int main(void){
	int ret,z;

	printf("Testing libblossom...\n");
	if( (ret = blossom_pthreads(tids,NULL,fxn,NULL)) ){
		fprintf(stderr,"blossom_pthreads returned %d\n",ret);
		return EXIT_FAILURE;
	}
	for(z = 0 ; z < tidcount ; ++z){
		void *arg;

		if( (ret = pthread_join(tids[z],&arg)) ){
			fprintf(stderr,"pthread_join returned %d\n",ret);
			return EXIT_FAILURE;
		}
		printf("Joined (Argument: %p)\n",arg);
	}
	printf("Tests succeeded.\n");
	return EXIT_SUCCESS;
}

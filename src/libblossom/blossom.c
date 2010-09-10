#include <libblossom/blossom.h>

int blossom_pthreads(pthread_t *tids,const pthread_attr_t *attr,
			void *(*fxn)(void *),void *arg){
	return pthread_create(tids,attr,fxn,arg);
}

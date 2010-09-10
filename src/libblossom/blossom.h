#ifndef LIBBLOSSOM_BLOSSOM
#define LIBBLOSSOM_BLOSSOM

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

int blossom_pthreads(pthread_t *,const pthread_attr_t *,void *(*)(void *),void *)
	__attribute__ ((visibility ("default")));

#ifdef __cplusplus
}
#endif

#endif

#ifndef LIBBLOSSOM_BLOSSOM
#define LIBBLOSSOM_BLOSSOM

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

typedef struct blossom_state {
	pthread_t *tids;
} blossom_state;

int blossom_pthreads(blossom_state *,const pthread_attr_t *,
			void *(*)(void *),void *)
	__attribute__ ((visibility ("default")))
	__attribute__ ((warn_unused_result));

#ifdef __cplusplus
}
#endif

#endif

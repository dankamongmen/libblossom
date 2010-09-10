#ifndef LIBBLOSSOM_BLOSSOM
#define LIBBLOSSOM_BLOSSOM

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

typedef struct blossom_state {
	pthread_t *tids;
	unsigned tidcount;
} blossom_state;

// Bloom tidcount threads. Any creation failure is a failure throughout.
int blossom_pthreads(blossom_state *,const pthread_attr_t *,
			void *(*)(void *),void *)
	__attribute__ ((visibility ("default")))
	__attribute__ ((warn_unused_result));

// Bloom tidcount threads per processing element. Any creation failure is a
// failure throughout.
int blossom_per_pe(blossom_state *,const pthread_attr_t *,
			void *(*)(void *),void *)
	__attribute__ ((visibility ("default")))
	__attribute__ ((warn_unused_result));

// Bloom tidcount threads per processing element, binding them to the
// appropriate processor. Any creation failure is a failure throughout.
int blossom_on_pe(blossom_state *,const pthread_attr_t *,
			void *(*)(void *),void *)
	__attribute__ ((visibility ("default")))
	__attribute__ ((warn_unused_result));

#ifdef __cplusplus
}
#endif

#endif

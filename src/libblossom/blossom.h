#ifndef LIBBLOSSOM_BLOSSOM
#define LIBBLOSSOM_BLOSSOM

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

// Primary input and output to blossoming functions. Tidcount is specified
// by the caller, while tids will be prepared by the library. After a
// successful call, blossom_free_state() ought be used to free any
// resources created by the library.
typedef struct blossom_state {
	pthread_t *tids;
	unsigned tidcount;
} blossom_state;

// Bloom tidcount threads. Any creation failure is a failure throughout.
int blossom_pthreads(blossom_state * __attribute__ ((non_null)),
			const pthread_attr_t *,
			void *(*)(void *) __attribute__ ((non_null)),
			void *)
	__attribute__ ((visibility ("default")))
	__attribute__ ((warn_unused_result));

// Bloom tidcount threads per processing element. Any creation failure is a
// failure throughout.
int blossom_per_pe(blossom_state * __attribute__ ((non_null)),
			const pthread_attr_t *,
			void *(*)(void *) __attribute__ ((non_null)),
			void *)
	__attribute__ ((visibility ("default")))
	__attribute__ ((warn_unused_result));

// Bloom tidcount threads per processing element, binding them to the
// appropriate processor. Any creation failure is a failure throughout.
int blossom_on_pe(blossom_state * __attribute__ ((non_null)),
			const pthread_attr_t *,
			void *(*)(void *) __attribute__ ((non_null)),
			void *)
	__attribute__ ((visibility ("default")))
	__attribute__ ((warn_unused_result));

// Free up the resources held in the blossom_state, though this does not
// perform any joining operation on the threads themselves.
void blossom_free_state(blossom_state *)
	__attribute__ ((visibility ("default")));

#ifdef __cplusplus
}
#endif

#endif

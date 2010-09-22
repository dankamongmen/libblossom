#ifndef LIBBLOSSOM_BLOSSOM
#define LIBBLOSSOM_BLOSSOM

#ifdef __cplusplus
extern "C" {
#endif

// libblossom automates the highly parallel spawning of threads. This is
// useful on machines with many processing elements, or NUMA machines where
// locality among processor subgroups is critical. Either some absolute
// number of threads can be spawned, or a number per processing element.

#include <pthread.h>

// Primary output from blossoming functions. After a successful call,
// blossom_free_state() ought be used to free any resources created by the
// library. In the meantime, these values can be used to control threads.
typedef struct blossom_state {
	pthread_t *tids;
	unsigned tidcount;
} blossom_state;

// Bloom tidcount threads. Any creation failure is a failure throughout.
int blossom_pthreads(unsigned,blossom_state *,const pthread_attr_t *,
			void *(*)(void *) __attribute__ ((nonnull)),void *)
	__attribute__ ((visibility ("default")))
	__attribute__ ((warn_unused_result));

// Bloom tidcount threads per processing element. Any creation failure is a
// failure throughout.
int blossom_per_pe(unsigned,blossom_state *,const pthread_attr_t *,
			void *(*)(void *) __attribute__ ((nonnull)),void *)
	__attribute__ ((visibility ("default")))
	__attribute__ ((warn_unused_result));

// Bloom tidcount threads per processing element, binding them to the
// appropriate processor. Any creation failure is a failure throughout.
int blossom_on_pe(unsigned,blossom_state *,const pthread_attr_t *,
			void *(*)(void *) __attribute__ ((nonnull)),void *)
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

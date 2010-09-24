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

typedef enum {
	// If any failure occurs, cancel and join all created threads. If
	// this is used with a pthread_attr_t specifying detached threads,
	// a "trampoline" will be set up which calls pthread_detach() in its
	// place. blossom_free_state() needn't be called if error is
	// returned in this case (though it is safe to do so); the
	// blossom_state structure will be zeroed out. No threads run the
	// argument function until all threads have been spawned.
	BLOSSOM_JOIN_ON_FAILURE,

	// Return an error on any failure, but go ahead and continue
	// attemting to spawn up through the desired number of threads.
	BLOSSOM_RUN_ON_FAILURE,
} blossom_init_flags;

typedef struct blossom_ctl {
	blossom_init_flags flags;
	unsigned tids;
} blossom_ctl;

// Bloom tidcount threads. Any creation failure is a failure throughout.
int blossom_pthreads(const blossom_ctl *,blossom_state *,const pthread_attr_t *,
			void *(*)(void *) __attribute__ ((nonnull)),void *)
	__attribute__ ((visibility ("default")))
	__attribute__ ((warn_unused_result));

// Bloom tidcount threads per processing element. Any creation failure is a
// failure throughout.
int blossom_per_pe(const blossom_ctl *,blossom_state *,const pthread_attr_t *,
			void *(*)(void *) __attribute__ ((nonnull)),void *)
	__attribute__ ((visibility ("default")))
	__attribute__ ((warn_unused_result));

// Bloom tidcount threads per processing element, binding them to the
// appropriate processor. Any creation failure is a failure throughout.
int blossom_on_pe(const blossom_ctl *,blossom_state *,const pthread_attr_t *,
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

#ifndef LIBBLOSSOM_SCHEDULE
#define LIBBLOSSOM_SCHEDULE

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#if defined(__linux__)
#include <sched.h>
#elif defined(__freebsd__)
#include <sys/param.h>
#include <sys/cpuset.h>
typedef cpuset_t cpu_set_t;
#else
#error "Need cpu_set_t definition for this platform" // solaris is psetid_t
#endif

// FreeBSD's cpuset.h (as of 7.2) doesn't provide CPU_COUNT, nor do older
// Linux setups (including RHEL5). This requires only CPU_{SETSIZE,ISSET}.
static inline unsigned
portable_cpuset_count(void){
        unsigned count = 0,cpu;
	cpu_set_t mask;
        
#if defined(__freebsd__)
        if(cpuset_getaffinity(CPU_LEVEL_CPUSET,CPU_WHICH_CPUSET,-1,
                                sizeof(mask),&mask) == 0){
#elif defined(__linux__)
        // We might be only a subthread of a larger application; use the
        // affinity mask of the thread which initializes us.
        if(pthread_getaffinity_np(pthread_self(),sizeof(mask),&mask) == 0){
#else
#error "Need cpu_set_t definition for this platform" // solaris is psetid_t
#endif
		for(cpu = 0 ; cpu < CPU_SETSIZE ; ++cpu){
			if(CPU_ISSET(cpu,&mask)){
				++count;
			}
		}
	}
        return count;
}       

#ifdef __cplusplus
}
#endif

#endif

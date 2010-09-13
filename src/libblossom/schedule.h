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
portable_cpuset_count(const cpu_set_t *mask){
        unsigned count = 0,cpu;
        
        for(cpu = 0 ; cpu < CPU_SETSIZE ; ++cpu){
                if(CPU_ISSET(cpu,mask)){
                        ++count;
                }       
        }       
        return count;
}       

#ifdef __cplusplus
}
#endif

#endif

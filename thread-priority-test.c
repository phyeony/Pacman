
// Source: https://wiki.linuxfoundation.org/realtime/documentation/howto/applications/application_base
/*
 * POSIX Real Time Example
 * using a single pthread as RT thread
 */

/*
    RT Linux Info:
    1. Priority
        https://wiki.linuxfoundation.org/realtime/documentation/howto/applications/application_base 
    2. Memory Locking & Stack memory
        https://wiki.linuxfoundation.org/realtime/documentation/howto/applications/memory#memory_locking
    


    View processes: 
    $ ps -alm
    $ top -H

    // https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8/html/monitoring_and_managing_system_status_and_performance/tuning-scheduling-policy_monitoring-and-managing-system-status-and-performance
        chrt -p <PID>

    Linux: https://medium.com/@chetaniam/a-brief-guide-to-priority-and-nice-values-in-the-linux-ecosystem-fb39e49815e0 
    PRI: Priority, used by the kernel for scheduling
        0 to 139 in which 0 to 99 for real-time and 100 to 139 for users.
        Lower = higher priority.

        See priority of a process:
        cat /proc/<pid>/sched | grep prio   # 120=default
        
        See priority of threads in a process
        cat /proc/<pid>/task/<threadid>/sched | grep prio
        --> cat /proc/<threadid>/sched                      # thread IDs not visible to ls, but they are there!
            policy:  0 = ?normal?; 1 = SCHED_FIFO; 2 = SCHED_RR

    NI: Nice value, user level 
        -20 to +19 where -20 is highest, 0 default and +19 is lowest.
        (smaller numbers mean less priority)
        
        Start command with set nice: 
            $ nice -n <nice-value> <command>
        Change nice level:
            $ renice -n <nice-value> -p <pid>
            (Must be root to NICE less than 0)

    "Priority_value = Nice_value + 20"
    PRI: 
        Kernel level reported priority; 0 to 139; 0-99 RT; 100-139 normal
        param.sched_priority = X; x==0  is low priority ==> kernel PRI 100
        param.sched_priority = X; x==99 is high priority ==> kernel PRI 0 (RT)

    Priority seems to be displayed as:
        0-39: process priority (lower = higher priority.)

    RT pthread create
        param.sched_priority = 60;
        ret = pthread_attr_setschedparam(&attr, &param);

        If set to 10, `top` shows PR "-11"
        If set to 60, `top` shows PR "-61"
        If set to 80, `top` shows PR "-81"  
        If set to 98, `top` shows PR "-99"  --> Kernel Priority 1
        If set to - 99, `top` shows PR "rt"   --> Kernel Priority 0		
        if set to 100 --> Fails setschedparam

        Watchdog is at -51!

*/

#define MY_PRIORITY 50  // 99=rt

#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

// #define _GNU_SOURCE
#include <unistd.h>
// #include <sys/types.h>   // For gettid()

int getThreadId()
{
    // pid_t tid = gettid();
    return (int)getpid();
}

void sleep_usec(long usec)
{
	struct timespec sleep_time;
	sleep_time.tv_sec = (usec / 1000000);
	sleep_time.tv_nsec = (usec % 1000000) * 1000;
	nanosleep(&sleep_time, NULL);
}

void sleep_msec(long msec)
{
	sleep_usec(msec * 1000);
}



void* thread_func(void* data)
{
    /* Do RT specific stuff here */
    for(int i = 0; i < 10000; i++) {
        printf("--> From high priority thread!\n");
        volatile unsigned int i = 0;
        while (i < 10000000) {
            i++;
        }
        sleep_msec(1000);
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    struct sched_param param;
    pthread_attr_t attr;
    pthread_t thread;
    int ret;

    /* Lock memory */
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
        printf("mlockall failed: %m\n");
        exit(-2);
    }

    /* Initialize pthread attributes (default values) */
    ret = pthread_attr_init(&attr);
    if (ret) {
        printf("init pthread attributes failed\n");
        goto out;
    }

    /* Set a specific stack size  */
    ret = pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
    if (ret) {
        printf("pthread setstacksize failed\n");
        goto out;
    }

    /* Set scheduler policy and priority of pthread */
    ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    // ret = pthread_attr_setschedpolicy(&attr, SCHED_RR);
    if (ret) {
        printf("pthread setschedpolicy failed\n");
        goto out;
    }
    param.sched_priority = MY_PRIORITY;
    ret = pthread_attr_setschedparam(&attr, &param);
    if (ret) {
        printf("pthread setschedparam failed\n");
        goto out;
    }
    /* Use scheduling parameters of attr */
    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (ret) {
        printf("pthread setinheritsched failed\n");
        goto out;
    }

    /* Create a pthread with specified attributes */
    ret = pthread_create(&thread, &attr, thread_func, NULL);
    if (ret) {
        printf("create pthread failed\n");
        goto out;
    }

    /* Do something to put us up TOP */
    for(int i = 0; i < 10000; i++) {
        printf("___ From low priority thread (ID: %d)!\n", getThreadId());
        volatile unsigned int i = 0;
        while (i < 10000000) {
            i++;
        }
        sleep_msec(1000);
    }


    /* Join the thread and wait until it is done */
    ret = pthread_join(thread, NULL);
    if (ret)
        printf("join pthread failed: %m\n");

out:
    return ret;
}

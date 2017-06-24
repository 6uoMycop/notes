## Get and measure elapsed process time

### times: get current process times
The *times* system call gets the user time and system time used by the current process and its children. User time is the time that the processor spent executing instructions in the process. System time is the time that the processor spent in the operating system (kernel) executing instructions on behalf of the process. The times system call has the following usage:
```C
#include <sys/times.h>
clock_t times(struct tms *buf);
```
Prior to calling *times*, you'll need to declare a variable of struct tms that the system call will fill in.
```C
struct tms
{
    clock_t tms_utime; /* user time */
    clock_t tms_stime; /* system time */
    clock_t tms_cutime; /* user time of children */
    clock_t tms_cstime; /* system time of children */
};
```
Note that all values are in `clock_t` units. All the time values are expressed in "clock ticks." This is an ever-incrementing count of the operating system's "ticks". We can obtain the frequency of these ticks via the *sysconf* function, which allows us to test and get various system configuration values at runtime. To find the clock frequency, we use:
```C
#include <unistd.h> 
sysconf(_SC_CLK_TCK); 
``` 
The call to *sysconf(_SC_CLK_TCK)* returns a long that indicates the number of ticks per second.

#### Example: reading current process times
This is a small program that does some busy work to use up a bit of CPU time and kernel resources (in the *kill_time* function) and then prints the time value in clock ticks.
```C
/* times: report times used by the current process */
/* Paul Krzyzanowski */

#include <stdio.h>      /* defines printf() */
#include <stdlib.h>     /* to define exit() */
#include <unistd.h>     /* for sysconf() */
#include <stdint.h>     /* used for casting clock_t to uintmax_t for printf */
#include <sys/times.h>      /* needed for the times() system call */
#include <sys/types.h>
#include <sys/stat.h>       /* used for open() */
#include <fcntl.h>      /* used for open() */

void kill_time(void);

int
main(int argc, char **argv) {
    struct tms t;   /* the time values will be placed into this struct */

    printf("tick frequency is: %lu\n", sysconf(_SC_CLK_TCK));

    kill_time();        /* do something to use up some time */
    if (times(&t) < 0) {
        perror("times");    /* error - print a message and exit */
        exit(1);
    }
    /* print the results */
    printf("user time: %ju ticks\n", (uintmax_t) t.tms_utime);
    printf("system time: %ju ticks\n", (uintmax_t) t.tms_stime);
    printf("chidren - user time: %ju ticks\n", (uintmax_t) t.tms_cutime);
    printf("chidren - system time: %ju ticks\n", (uintmax_t) t.tms_cstime);

    exit(0);
}

void
kill_time(void) {
    long long i, j; /* use these in a loop to kill time */
    int fd;
    char buf[2048];

    printf("doing some cpu wasting stuff\n");
    for (i=0; i<100000000; i++)
        j += i;

    printf("doing some kernel wasting stuff\n");
    /* do some stuff to waste system time */
    if ((fd = open("/dev/urandom", O_RDONLY)) < 0) {
        perror("open");
        exit(1);
    }
    for (i=0; i < 1000; i++) 
        read(fd, buf, 2048);
    close(fd);
}
```
### Measure elapsed time between two points in your code
The times system call returns the current tick count. This is an ever-incrementing number from some arbitrary point in the past. While this on its own is not useful, we can use it to mark a starting and ending time. The difference between these two times is the elapsed time.

#### Example: measuring elapsed time
In the last example, we ignored the return value from a call to *times*. In this program, we ignore the `struct tms` parameter to *times* but record a start time and a stop time. Between these two times, it calls *sleep(2)* to sleep for two seconds. It then prints the result in ticks and in seconds. To convert ticks to seconds, you simply divide the elapsed tick count by the frequency obtained from *sysconf(_SC_CLK_TCK)*.
```C
/* times: use times() to get the elapsed time */
/* Paul Krzyzanowski */

#include <stdio.h> /* declares printf() */
#include <stdlib.h> /* declares exit() */
#include <unistd.h> /* declares sysconf() */
#include <stdint.h> /* used for casting clock_t to uintmax_t for printf */
#include <sys/times.h> /* needed for the times() system call */

int
main(int argc, char **argv) {
    struct tms t; /* parameter to times() - we'll ignore it */
    unsigned long freq; /* clock frequency */
    uintmax_t start; /* starting tick value */
    uintmax_t end; /* ending tick value */

    printf("sizeof(clock_t) = %lu\n", sizeof(clock_t));
    printf("tick frequency is: %lu\n", freq=sysconf(_SC_CLK_TCK));

    if ((start = times(&t)) < 0) { /* mark the start time */
        perror("times"); /* error - print a message and exit */
        exit(1);
    }
    sleep(2); /* sleep for two seconds */
    if ((end = times(&t)) < 0) { /* mark the end time */
        perror("times"); /* error - print a message and exit */
        exit(1);
    }
    printf("elapsed time: %ju - %ju = %ju ticks\n", end, start, end-start);
    printf("elapsed time: %5.2f seconds\n", (double)(end-start)/freq);
    exit(0);
}
```
### The clock function 
The clock library function (not system call) provides an analogous approach to the return value of times to mark start and end times. Clock takes no parameter and returns an estimate of the processor time used by the running process. If you try the above program that sleeps for two seconds with clock, you will get a value of 0 elapsed seconds. Basically, the call to sleep put the process to sleep and there was no CPU time used on behalf of the process during that time.


While *clock* returns its result in the same `clock_t` data type as *times*, the scale is different. The POSIX requires that the result be scaled by a value CLOCKS_PER_SEC, which is always defined as one million (1000000), regardless of the actual clock resolution.
```C
/* times: use clock() to get the elapsed time */

#include <stdio.h>      /* declares printf() */
#include <stdlib.h>     /* declares exit() */
#include <stdint.h>     /* used for casting clock_t to uintmax_t for printf */
#include <time.h>       /* needed for the clock() library function */

int
main(int argc, char **argv) {
    unsigned long freq; /* clock frequency */
    uintmax_t start;    /* starting tick value */
    uintmax_t end;      /* ending tick value */

    start = clock();    /* mark the start time */
    sleep(2);   /* sleep for two seconds */
    end = clock();      /* mark the end time */

    printf("elapsed time: %ju - %ju = %ju ticks\n", end, start, end-start);
    printf("elapsed time: %8.6f seconds\n", (double)(end-start)/CLOCKS_PER_SEC);
    exit(0);
}
```
### The gettimeofday system call 
This function, as its name implies, measures the time of day. It returns its value as the number of seconds and microseconds since the Epoch, which is defined as midnight (0:00) January 1, 1970 UTC (Greenwich Mean Time). This value is returned in the first parameter, which is a pointer to a structure that contains two fields: seconds and microseconds.
```C
/* times: use gettimeofday() to get the elapsed time */
/* Paul Krzyzanowski */

#include <stdio.h>      /* declares printf() */
#include <stdlib.h>     /* declares exit() */
#include <stdint.h>     /* used for casting clock_t to uintmax_t for printf */
#include <sys/time.h>       /* needed for the gettimeofday() system call */

int
main(int argc, char **argv) {
    struct timeval start;   /* starting time */
    struct timeval end; /* ending time */
    unsigned long e_usec;   /* elapsed microseconds */

    gettimeofday(&start, 0);    /* mark the start time */
    sleep(2);       /* sleep for 2 seconds */
    gettimeofday(&end, 0);      /* mark the end time */

    /* now we can do the math. timeval has two elements: seconds and microseconds */
    e_usec = ((end.tv_sec * 1000000) + end.tv_usec) - ((start.tv_sec * 1000000) + start.tv_usec);

    printf("elapsed time: %lu microseconds\n", e_usec);
    exit(0);
}
```

## Use Linux's high resolution clock
### Clock_gettime
#### Example
This example marks the start time by getting the value of CLOCK_MONOTONIC. The process then sleeps for a second and marks the stop time by getting the value of CLOCK_MONOTONIC a second time. It then does the same thing again but uses CLOCK_PROCESS_CPUTIME_ID. Since the CPU is not used for the time that the process is sleeping, the results are substantially shorter.
```C
/*
    gettime - get time via clock_gettime
    N.B.: OS X does not support clock_gettime

    Paul Krzyzanowski
*/

#include <stdio.h>  /* for printf */
#include <stdint.h> /* for uint64 definition */
#include <stdlib.h> /* for exit() definition */
#include <time.h>   /* for clock_gettime */

#define BILLION 1000000000L

int localpid(void) {
    static int a[9] = { 0 };
    return a[0];
}

main(int argc, char **argv)
{
    uint64_t diff;
    struct timespec start, end;
    int i;

    /* measure monotonic time */
    clock_gettime(CLOCK_MONOTONIC, &start); /* mark start time */
    sleep(1);   /* do stuff */
    clock_gettime(CLOCK_MONOTONIC, &end);   /* mark the end time */

    diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    printf("elapsed time = %llu nanoseconds\n", (long long unsigned int) diff);

    /* now re-do this and measure CPU time */
    /* the time spent sleeping will not count (but there is a bit of overhead */
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);    /* mark start time */
    sleep(1);   /* do stuff */
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);      /* mark the end time */

    diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    printf("elapsed process CPU time = %llu nanoseconds\n", (long long unsigned int) diff);

    exit(0);
}
```

### tsc
The x86 "timestamp counter", or TSC, is a 64-bit register on each processor that increases monotonically. It increments once every CPU clock-cycle, starting from 0 when power is turned on. It won't overflow for at least ten years. Unprivileged programs (ring3) normally can access it via the `rdtsc` instruction.

So how much time is one cycle? You can compute the cycle time by counting the number of cycles in a known time interval.
```C
#ifndef TIMER_H_
#define TIMER_H_

#include <inttypes.h>

#define UINT32_T uint32_t
#define UINT64_T uint64_t

#define HRT_CALIBRATE(freq) do {  \
  static volatile HRT_TIMESTAMP_T t1, t2; \
  static volatile UINT64_T elapsed_ticks, min = (UINT64_T)(~0x1); \
  int notsmaller=0; \
  while(notsmaller<3) { \
    HRT_GET_TIMESTAMP(t1); \
     sleep(1);  \
    /* nanosleep((struct timespec[]){{0, 10000000}}, NULL); */ \
    HRT_GET_TIMESTAMP(t2); \
    HRT_GET_ELAPSED_TICKS(t1, t2, &elapsed_ticks); \
    notsmaller++; \
    if(elapsed_ticks<min) { \
      min = elapsed_ticks; \
      notsmaller = 0; \
    } \
  } \
  freq = min; \
} while(0);

#define HRT_INIT(freq) HRT_CALIBRATE(freq)

#define HRT_TIMESTAMP_T x86_64_timeval_t

#define HRT_GET_TIMESTAMP(t1)  __asm__ __volatile__ ("rdtsc" : "=a" (t1.l), "=d" (t1.h));

#define HRT_GET_ELAPSED_TICKS(t1, t2, numptr)   *numptr = (((( UINT64_T ) t2.h) << 32) | t2.l) - \
                                                          (((( UINT64_T ) t1.h) << 32) | t1.l);

typedef struct {
    UINT32_T l;
    UINT32_T h;
} x86_64_timeval_t;

/* global timer frequency in Hz */
extern unsigned long long g_timerfreq;

#define HRT_GET_USEC(ticks) 1e6/*1e4*/*(double)ticks/(double)g_timerfreq

#endif /* TIMER_H_ */
```
```C
unsigned long long g_timerfreq;

int main(int argc, char const *argv[])
{
    HRT_INIT(g_timerfreq);

    HRT_TIMESTAMP_T t1, t2;
    HRT_GET_TIMESTAMP(t1);
    /* do something */
    HRT_GET_TIMESTAMP(t2);
    
    uint64_t ticks;
    HRT_GET_ELAPSED_TICKS(t1, t2, &ticks);

    printf("%lf\n", HRT_GET_USEC(ticks));
    
    return 0;
}
```

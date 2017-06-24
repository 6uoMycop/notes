## Affinities
- taskset command
   - sudo taskset -c 4 33187 # pid 33187 runs on core 4
   - sudo taskset -c 0,2,3 ./a.out

- sched_setaffinity
   
   ```
   +extern int vcpu_affinity[];
    static void qemu_kvm_start_vcpu(CPUState *cpu)
    {
   
        char thread_name[VCPU_THREAD_NAME_SIZE];
   -
   +    cpu_set_t cpuset;
   + 
        cpu->thread = g_malloc0(sizeof(QemuThread));
        cpu->halt_cond = g_malloc0(sizeof(QemuCond));
        qemu_cond_init(cpu->halt_cond);
        snprintf(thread_name, VCPU_THREAD_NAME_SIZE, "CPU %d/KVM",
                 cpu->cpu_index);
        qemu_thread_create(cpu->thread, thread_name, qemu_kvm_cpu_thread_fn,
                           cpu, QEMU_THREAD_JOINABLE);
   +    if (vcpu_affinity[cpu->cpu_index] != -1) {
   +	    printf("QEMU INFO:: Pinning vcpu %d to Physical CPU %d\n",cpu->cpu_index, vcpu_affinity[cpu->cpu_index]);
   +	    CPU_ZERO(&cpuset);
   +	    CPU_SET(vcpu_affinity[cpu->cpu_index], &cpuset); 
   +	    pthread_setaffinity_np((cpu->thread)->thread, sizeof(cpu_set_t), &cpuset);
   +    }
   +     
        while (!cpu->created) {
            qemu_cond_wait(&qemu_cpu_cond, &qemu_global_mutex);
        }
   ```

## The /proc filesystem
The `/proc` filesystem contains a illusionary filesystem. It does not exist on a disk. Instead, the kernel creates it in memory. It is used to provide information about the system (originally about processes, hence the name). 

## Process Time
Process time is the amount of CPU time used by a process since it was created. For recording purposes, the kernel separates CPU time into the following two components

- *User CPU time* is the amount of time spent executing in user mode.
- *System CPU time* is amount of time spent executing in kernel mode. This is the time that the kenel spends executing system calls or performing other tasks on behalf of the program (e.g., servicing page faults).

Sometimes, we refer to process time as the *total CPU time* consumed by the process.

When we run a program from the shell, we can use the `time` command to obtain both process time values, as well as the real time required to run the program.
```
$ time ./myprog
real 0m4.84s
user 0m1.030s
sys  0m3.43s
```
The `times()` system call retrieves process time information, returning it in the structure pointed to by `buf()`.
```
#include <sys/times.h>

clock_t times(struct tms *buf);
```
This `tms` structure pointed to by *buf* has the following form:
```
struct tms {
	clock_t tms_utime;
	clock_t tms_stime;
	clock_t tms_cutime;
	clock_t tms_cstime;
}
```
The first two fields of the `tms` structure return the user and system components of CPU time used so far by the calling process. The last two fields return information about the CPU time used by all child processes that have terminated and for which the parent (i.e., the caller of `times()`) has done a `wait()` system call.

The `clock_t` data type used to type the four fields of the tms structure is an integer type that measures time in uints called *clock ticks*. We can call `sysconf(_SC_CLK_TCK)` to obtain the number of clock ticks per second, and then divide a `clock_t` value by this number to convert to seconds.

The `clock()` function provides a simpler interface for retrieving the process time. It returns a single value that measures the toal (i.e., user plus system) CPU time used by the calling process.

The value returned by `clock()` is measured in units of `CLOCKS_PER_SEC`, so we must divide by this value to arrive at the number of seconds of CPU time used by the process.

### Example program
```
#include <sys/times.h>
#include <time.h>
#include "tlpi_hdr.h"

static void             /* Display 'msg' and process times */
displayProcessTimes(const char *msg)
{
	struct tms t;
	clock_t clockTime;
	static long clockTicks = 0;

	if (msg != NULL)
		printf("%s\n", msg);

	if (clockTicks == 0) {
		clockTicks = sysconf(_SC_CLK_TCK);
		if (clockTicks == -1) {
			errExit("sysconf");
		}
	}

	clockTime = clock();
	if (clockTime == -1)
		errExit("clock");

	printf("        clock() returns: %ld clocks-per-sec (%.2f secs)\n", (long)clockTime, (double)clockTime / CLOCKS_PER_SEC);

	if (times(&t) == -1)
		errExit("times");
	printf("        times() yields: user CPU=%.2f; system CPU: %.2f\n", (double)t.tms_utime / clockTicks, (double)t.tms_stime / clockTicks);
}

int
main(int argc, char const *argv[])
{
	int numCalls, j;

	printf("CLOCKS_PER_SEC = %ld   sysconf(_SC_CLK_TCK)=%ld\n\n", (long)CLOCKS_PER_SEC, sysconf(_SC_CLK_TCK));

	displayProcessTimes("At program start:\n");

	numCalls = (argc > 1) ? getInt(argv[1], GN_GT_o, "num-calls") : 100000000;
	for (j = 0; j < numCalls; j++)
		(void)getppid();

	displayProcessTimes("After getppid() loop:\n");

	exit(EXIT_SUCCESS);
	return 0;
}
```
## TSC background
The x86 "timestamp counter", or TSC, is a 64-bit register on each processor that increases monotonically. Historically, TSC incremented every processor cycle, but on recent processors, it increases at a constant rate even if the processor changes frequency (for example, to reduce processor power usage).  TSC is known by x86 programmers as the fastest, highest-precision measurement of the passage of time so it is often used as a foundation for performance monitoring.

On most older SMP and early multi-core machines, TSC was not synchronized between processors. Thus if an application were to read the TSC on one processor, then was moved by the OS to another processor, then read TSC again, it might appear that "time went backwards".  This loss of monotonicity resulted in many obscure application bugs when TSC-sensitive apps were ported from a uniprocessor to an SMP environment. On some more recent generations of multi-core machines, especially multi-socket multi-core machines, the TSC was synchronized but if one processor were to enter certain low-power states, its TSC would stop, destroying the synchrony and again causing obscure bugs.  This reinforced decisions to avoid use of TSC altogether.  On the most recent generations of multi-core machines, however, synchronization is provided across all processors in all power states, even on multi-socket machines, and provide a flag that indicates that TSC is synchronized and "invariant". Thus TSC is once again useful for applications, and even newer operating systems are using and depending upon TSC for critical timekeeping tasks when running on these recent machines.

The emergence of virtualization once again complicates the usage of TSC. When features such as save/restore or live migration are employed, a guest OS and all its currently running applications may be invisibly transported to an entirely different physical machine. While TSC may be "safe" on one machine, it is essentially impossible to precisely synchronize TSC across a data center or even a pool of machines. As a result, when run in a virtualized environment, rare and obscure "time going backwards" problems might once again occur for those TSC-sensitive applications. Worse, if a guest OS moves from, for example, a 3GHz machine to a 1.5GHz machine, attempts by an OS/app to measure time intervals with TSC may without notice be incorrect by a factor of two.

## Time Keeping in Xen
```C
typedef struct vcpu_time_info {

    /*
     * Updates to the following values are preceded and followed by an
     * increment of 'version'. The guest can therefore detect updates by
     * looking for changes to 'version'. If the least-significant bit of
     * the version number is set then an update is in progress and the guest
     * must wait to read a consistent set of values.
     */
    uint32_t version;
    uint32_t pad0;
    uint64_t tsc_timestamp;   /* TSC at last update of time vals.  */
    uint64_t system_time;     /* Time, in nanosecs, since boot.    */
    /*
     * Current system time:
     *   system_time + ((tsc - tsc_timestamp) << tsc_shift) * tsc_to_system_mul
     * CPU frequency (Hz):
     *   ((10^9 << 32) / tsc_to_system_mul) >> tsc_shift
     */
    uint32_t tsc_to_system_mul;
    int8_t   tsc_shift;
    int8_t   pad1[3];
} vcpu_time_info_t; /* 32 bytes */
```
This is essentially a mechanism which allows the guest to use direct access to the native clock plus scaling and offset factors exposed from the hypervisor via shared memory to come up with a PV timesource which can be read without needing to trap to the hypervisor.

In Linux it is implemented in `arch/x86/kernel/pvclock.c` and is shared by a variety of hypervisors.

I'm talking about the "tick" type of time source here, there is also a pv wallclock (i.e. date and time) interface, but I don't think that's what Linux's clocksource is about. Hopefully I've not confused the two (I often do).
```C
typedef struct shared_info {
    
    ...
    /*
     * Wallclock time: updated only by control software. Guests should base
     * their gettimeofday() syscall on this wallclock-base value.
     */
    uint32_t wc_version;      /* Version counter */
    uint32_t wc_sec;          /* Secs  00:00:00 UTC, Jan 1, 1970.  */
    uint32_t wc_nsec;         /* Nsecs 00:00:00 UTC, Jan 1, 1970.  */
    ...

} shared_info_t;
```

```
-------------------       ---------------------------            ---------------------
|shared_info_t    |       |vcpu_info_t              |--          |arch_vcpu_info_t   |
|                 |       |                         | |          |                   |
|                 |       |  evtchn_upcall_pending  | |          |               cr2 |
|    vcpu_info[]--|---    |                         | |  ------->|                   |
|                 |  |    |     evtchn_upcall_mask  | |  |       |               pad |
|                 |  |    |                         | |  |       ---------------------
| evtchn_pending  |  |    |     evtchn_pending_sel  | |  |
|                 |  ---->|                         | |  |       ----------------------------
|                 |       |                   arch--|-|---       |vcpu_time_info_t          |
|    evtchn_mask  |       |                         | |          |                          |
|                 |       |                   time--|-|---       |                 version  |
|                 |       --------------------------- |  |       |                          |
|     wc_version  |         |-------------------------   |       |                    pad0  |
|                 |                                      |       |                          |
|                 |                                      ------->|           tsc_timestamp  |
|         wc_sec  |                                              |                          |
|                 |                                              |             system_time  |
|                 |        ------------------------------        |                          |
|                 |        |arch_shared_info_t          |        |       tsc_to_system_mul  |
|        wc_nsec  |  ----->|                            |        |                          |
|                 |  |     |                   max_pfn  |        |               tsc_shift  |
|                 |  |     |                            |        |                          |
|           arch--|---     |pfn_to_mfn_frame_list_list  |        |                    pad1  |
-------------------        -----------------------------         ----------------------------
```

```C
uint64_t tscToNanoseconds(uint64_t tsc, struct vcpu_time_info *timeinfo)
{
    return (tsc << timeinfo->tsc_shift) * timeinfo->tsc_to_system_mul;
}
```
Calling this with the value extracted from the TSC register (using the RDTSC instruction) only gives you the number of nanoseconds until some arbitrary point in the past, which is not particularly useful. It can however, be used to determine how much time has elapsed since the system time stamp was written. Every time this is modified by the hypervisor, the TSC value is also written (to the `tsc_timestamp` field of the `vcpu_time_info_t` structure). From this, you can calculate the current system.

After you have the current system time, you add this to the real (wall) time at system time zero, giving you the current time. The wall clock time at system time zero is stored in the `wc_sec` and `wc_nsec` fields of the shared info structure.

> Do We Need System Time?
>
> It might seem that you don't need system time. If you have the wall clock time at system boot / resume, and the Time Stamp Counter when this happened, you ought to be able to calculate the current system. In theory, this is the case. In practice, the TSC rate accuracy is limited by the accuracy of the timing circuitry in the system, which is typically prone to a small amount of variable skew. Although it gives a very fine-grained value, it does not give a very accurate one. A system clock using just the TSC value will experience drift. To combat this, this Domain 0 guest is expected to run at NTP client or update its clock from a high-resolution time source. This value is then used to update the system time, reducing drift in the guest.

Implementing the POSIX `gettimeofday()` function requires the use of the shared memory page, the Time-Stamp Counter, and some simple calculations. This section will describe the implementation of the function.

To read these, we need to examine the version value of the `vcpu_time_info_t` structure. If the lowest bit of this is 1, then the timer values are being updated, and so we simply spin until this is not the case. When it is a 0, we keep a copy of the value, attempt to read the wall clock values, and then test the version again. If the version number has not changed, we can proceed; otherwise, we loop.

At first glance, it appears that checking the value of the version is not required. The reason it exists it that a guest may be preempted by the hypervisor and have the time values updated while it is reading them.

```C
int gettimeofday(struct timeval *tp, struct timezone *tzp)
{
    uint64_t tsc;
    /* Get the time values from the shared info page */
    uint32_t version, wc_version;
    uint32_t seconds, nanoseconds, system_time;
    uint64_t old_tsc;
    /* Loop until we can read all required values from the same update */
    do
    {
        /* Spin if the time value is being updated*/
        do
        {
            wc_version = shared_info->wc_version;
            version = shared_info->cpu_info[0].time.version;
        } while (
                 version & 1 == 1
                 ||
                 wc_version & 1 == 1);
        /* Read the value */
        seconds = shared_info->wc_sec;
        nanoseconds = shared_info->wc_nsec;
        system_time = shared_info->cpu_info[0].time.system_time;
        old_tsc = shared_info->cpu_info[0].time.tsc_timestamp;
    } while (
             version != shared_info->cpu_info[0].time.version
             ||
             wc_version != shared_info->wc_version
             );
    /* Get the current TSC value */
    RDTSC(tsc);
    /* Get the number of elapsed cycles */
    tsc -= old_tsc;
    /* Update the system time */
    system_time += NANOSECONDS(tsc);
    /* Update the nanosecond time */
    nanoseconds += system_time;
    /* Move complete seconds to the second counter */
    seconds += nanoseconds / 1000000000;
    nanoseconds = nanoseconds % 1000000000;
    /* Return second and millisecond values */
    tp->tv_sec = seconds;
    tp->tv_usec = nanoseconds * 1000;
    return 0;
}
```

## TSC background
The x86 "timestamp counter", or TSC, is a 64-bit register on each processor that increases monotonically. Historically, TSC incremented every processor cycle, but on recent processors, it increases at a constant rate even if the processor changes frequency (for example, to reduce processor power usage).  TSC is known by x86 programmers as the fastest, highest-precision measurement of the passage of time so it is often used as a foundation for performance monitoring.

On most older SMP and early multi-core machines, TSC was not synchronized between processors. Thus if an application were to read the TSC on one processor, then was moved by the OS to another processor, then read TSC again, it might appear that "time went backwards".  This loss of monotonicity resulted in many obscure application bugs when TSC-sensitive apps were ported from a uniprocessor to an SMP environment. On some more recent generations of multi-core machines, especially multi-socket multi-core machines, the TSC was synchronized but if one processor were to enter certain low-power states, its TSC would stop, destroying the synchrony and again causing obscure bugs.  This reinforced decisions to avoid use of TSC altogether.  On the most recent generations of multi-core machines, however, synchronization is provided across all processors in all power states, even on multi-socket machines, and provide a flag that indicates that TSC is synchronized and "invariant". Thus TSC is once again useful for applications, and even newer operating systems are using and depending upon TSC for critical timekeeping tasks when running on these recent machines.

The emergence of virtualization once again complicates the usage of TSC. When features such as save/restore or live migration are employed, a guest OS and all its currently running applications may be invisibly transported to an entirely different physical machine. While TSC may be "safe" on one machine, it is essentially impossible to precisely synchronize TSC across a data center or even a pool of machines. As a result, when run in a virtualized environment, rare and obscure "time going backwards" problems might once again occur for those TSC-sensitive applications. Worse, if a guest OS moves from, for example, a 3GHz machine to a 1.5GHz machine, attempts by an OS/app to measure time intervals with TSC may without notice be incorrect by a factor of two.

## Time Keeping in Xen
```
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

This is the "tick" type of time source. There is also a pv wallclock (i.e. date and time) interface:
```
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
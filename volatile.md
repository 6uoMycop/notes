## Volatile
The reason for having this type qualifier is mainly to do with the problems that are encountered in real-time or embedded systems programming using C. Imagine that you are writing code that controls a hardware device by placing appropriate values in hardware registers at known absolute addresses.

Let's imagine that the device has two registers, each 16 bits long, at ascending memory addresses; the first one is the control and status register (csr) and the second is a data port. The traditional way of accessing such a device is like this:
```
/* Standard C example but without const or volatile */
/*
* Declare the device registers
* Whether to use int or short
* is implementation dependent
*/

struct devregs{
        unsigned short  csr;    /* control & status */
        unsigned short  data;   /* data port */
};

/* bit patterns in the csr */
#define ERROR   0x1
#define READY   0x2
#define RESET   0x4

/* absolute address of the device */
#define DEVADDR ((struct devregs *)0xffff0004)

/* number of such devices in system */
#define NDEVS   4

/*
* Busy-wait function to read a byte from device n.
* check range of device number.
* Wait until READY or ERROR
* if no error, read byte, return it
* otherwise reset error, return 0xffff
*/
unsigned int read_dev(unsigned devno){

        struct devregs *dvp = DEVADDR + devno;

        if(devno >= NDEVS)
                return(0xffff);

        while((dvp->csr & (READY | ERROR)) == 0)
                ; /* NULL - wait till done */

        if(dvp->csr & ERROR){
                dvp->csr = RESET;
                return(0xffff);
        }

        return((dvp->data) & 0xff);
}
```
However, a major problem with previous C compilers would be in the while loop which tests the status register and waits for the `ERROR` or `READY` bit to come on. Any self-respecting optimizing compiler would notice that the loop tests the same memory address over and over again. It would almost certainly arrange to reference memory once only, and copy the value into a hardware register, thus speeding up the loop. This is, of course, exactly what we don't want; this is one of the few places where we must look at the place where the pointer points, every time around the loop.

Because of this problem, most C compilers have been unable to make that sort of optimization in the past. To remove the problem (and other similar ones to do with when to write to where a pointer points), the keyword `volatile` was introduced. It tells the compiler that the object is subject to sudden change for reasons which cannot be predicted from a study of the program itself, and forces every reference to such an object to be a genuine reference.

### Example
- Important for concurrency (software and hardware!)
- Usually, the compiler has some freedom where to store the contents of variables
- Dependent on this storage location, concurrent updates mighe be seen or not
- Example
  ```
  int flag;
  void foo(void) {
      if (flag)
	      do_something;

      /* flag modified by another thread */

      if (flag)
          do_another_thing;
  }
  ```
  - Flag modified by another thread between two accesses
  - Assume compiler keeps `flag` in register in `foo`
  - Reasonable optimization to save memory accesses
  - Concurrent update invisible
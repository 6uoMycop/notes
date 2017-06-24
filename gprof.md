# gprof
## Introduction to Profiling
Profiling has several steps:
- You must compile and link your program with profiling enabled.
- You must execute your program to generate a profile data file.
- You must run `gprof` to analyze the profile data.

## Compiling a Program for Profiling
The first step in generating profile information for your program is to compile and link it with profiling enabled.

To compile a source file for profiling, specify the `'-pg'` option when you run the compiler.

To link the program for profiling, if you use a compiler such as `cc` to do the linking, simply specify `'-pg'` in addition to your usual options. The same option, `'-pg'`, alters either compilation or linking to do what is necessary for profiling. Here are examples:
```
cc -g -c myprog.c utils.c -pg
cc -o myprog myprog.o utils.o -pg
```
The `'-pg'` option also works with a command that both compiles and links:
```
cc -o myprog myprog.c utils.c -g -pg
```
If you run the linker `ld` directly instead of through a compiler such as `cc`, you may have to specify a profiling startup file `gcrt0.o` as the first input file instead of the usual startup file `crt0.o`.

In addition, you would probably want to specify the profiling C library, `libc_p.a`, by writing `'-lc_p'` instead of the usual `'-lc'`. This is not absolutely necessary, but doing this gives you number-of-calls information for standard library functions such as read and open. For example:
```
ld -o myprog /lib/gcrt0.o myprog.o utils.o -lc_p
```

## Executing the Program
Once the program is compiled for profiling, you must run it in order to generate the information that gprof needs. Simply run the program as usual, using the normal arguments, file names, etc. The program should run normally, producing the same output as usual. It will, however, run somewhat slower than normal because of the time spent collecting and writing the profile data.

Your program will write the profile data into a file called `gmon.out` just before exiting. If there is already a file called `gmon.out`, its contents are overwritten.

## gprof Command Summary
After you have a profile data file gmon.out, you can run gprof to interpret the information in it. The gprof program prints a flat profile and a call graph on standard output. Typically you would redirect the output of gprof into a file with `'>'`.

You run gprof like this:
```
gprof options [executable-file [profile-data-files...]] [> outfile]
```
Here square-brackets indicate optional arguments.

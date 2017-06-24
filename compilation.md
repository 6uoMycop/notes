## How the Compiler Works
The sequence of commands executed by a single invocation of `gcc` consists of the following stages:
1. Preprocessing
2. Compilation
3. Assembly
4. Linking

**Preprocessor**

The first stage of the compilation process is the use of a *preprocessor* program to expand the macros and include the header files. The first preprocessor directive in our program above is `#include <stdio.h>` which instructs the processor to include the header file `stdio.h` into our source. This preprocessor phase is also responsible for removing the comments. The following command can be used to create the preprocessed file `abc.i` from the source file `abc.c`.
```
$ gcc -E abc.c > abc.i
```
or
```
$ cpp abc.c > abc.i
```
The result is an intermediate file `abc.i` which contains the source code with all macros expanded.

**Compiler**

The next stage of the process is the actual compilation of the preprocessed source code to assembly language, for the specific type of processor. The command line option `-S` instructs `gcc` to convert the preprocessed C source code to assembly language without creating an object file.
```
$ gcc -S abc.i
```
This will create a file `abc.s` which contains the equivalent assembly code the C program. The content of the acbc.s is as follows:
```
     .file  "abc.c"
     .section  .rodata
.LC0:
     .string "test"
     .text
.globl main
     .type   main, @function
main:
       leal    4(%esp), %ecx
       andl    $-16, %esp
       pushl   -4(%ecx)
       pushl   %ebp
       movl    %esp, %ebp
       pushl   %ecx
       subl    $4, %esp
       movl    $.LC0, (%esp)
       call    printf
       movl    $0, %eax
       addl    $4, %esp
       popl    %ecx
       popl    %ebp
       leal    -4(%ecx), %esp
       ret
```

**Assembler**

The job of the assembler is to convert the assembly language into machine code and generate an object file. When there are calls to external functions in the assembly source file, the assembler leaves the address of the external functions undefined, to be filled in by the linker.

The assembler can be invoked with the following command line:
```
$ as abc.s -o abc.o
```
The file `abc.o` will be containing the object code or we can say the machine instruction for the C program `abc.c`.

**Linker**

The last stage of the compilation process is the linking of all object files to create an executable file. In general an executable file is required by many external functions which may be present in different libraries. So, the actual command used for linking is quite complicated. For example in our program if we want to link then the command is:
```
$ ld -dynamic-linker /lib/ld-linux.so.2 /usr/lib/crt1.o /usr/lib/crti.o /usr/lib/gcc-lib/i686/3.3.1/crtbegin.o -L/usr/lib/gcc-lib/i686/3.3.1 hello.o -lgcc -lgcc_eh -lc -lgcc -lgcc_eh /usr/lib/gcc-lib/i686/3.3.1/crtend.o /usr/lib/crtn.o
```

But there is no need to write such a big command because all these processes are handled by `gcc` transparently.

Instead of the above big command, type:
```
$ gcc abo.o
```
This links the object file `abo.o` to the C standard library, and produces and executable file `a.out`.

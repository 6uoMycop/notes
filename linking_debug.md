## gdb
```
(gdb) help x
Examine memory: x/FMT ADDRESS.
ADDRESS is an expression for the memory address to examine.
FMT is a repeat count followed by a format letter and a size letter.
Format letters are x(hex), i(instruction), c(char).
The specified number of objects of the specified size are printed
according to the format.

(gdb) list
1	#include <unistd.h>
2	#include <stdio.h>
3	
4	int main()
5	{
6	  char * args[] = {"/bin/ls", NULL};
7	
8	  printf("pid: %d\n", getpid());
9	
10	  //sleep(60);
(gdb) break main
Breakpoint 1 at 0x4005c5: file test.c, line 6.
(gdb) run
Starting program: ./a.out 

Breakpoint 1, main () at test.c:6
6	  char * args[] = {"/bin/ls", NULL};
(gdb) disassemble main
Dump of assembler code for function main:
   0x00000000004005bd <+0>:	push   %rbp
   0x00000000004005be <+1>:	mov    %rsp,%rbp
   0x00000000004005c1 <+4>:	sub    $0x10,%rsp
=> 0x00000000004005c5 <+8>:	movq   $0x400684,-0x10(%rbp)
   0x00000000004005cd <+16>:	movq   $0x0,-0x8(%rbp)
   0x00000000004005d5 <+24>:	callq  0x400480 <getpid@plt>
   0x00000000004005da <+29>:	mov    %eax,%esi
   0x00000000004005dc <+31>:	mov    $0x40068c,%edi
   0x00000000004005e1 <+36>:	mov    $0x0,%eax
   0x00000000004005e6 <+41>:	callq  0x400490 <printf@plt>
   0x00000000004005eb <+46>:	lea    -0x10(%rbp),%rax
   0x00000000004005ef <+50>:	mov    %rax,%rsi
   0x00000000004005f2 <+53>:	mov    $0x400684,%edi
   0x00000000004005f7 <+58>:	callq  0x4004c0 <execv@plt>
   0x00000000004005fc <+63>:	leaveq 
   0x00000000004005fd <+64>:	retq   
End of assembler dump.
(gdb) x/10c 0x400684
0x400684:	47 '/'	98 'b'	105 'i'	110 'n'	47 '/'	108 'l'	115 's'	0 '\000'
0x40068c:	112 'p'	105 'i'
```

## Position-Independent Code (PIC)
mymath.c
```
/* gcc -shared -fPIC -o libmymath.so mymath.c */

int global_var = 10;

float cubed(float a)
{
	return a*a*a;
}

float do_math(float a)
{
	float ret = cubed(a);

	ret += global_var;

	return ret;
}
```

main.c
```
/* gcc -o p main.c ./libmymath.so */

#include <stdio.h>

float do_math(float a);

int main(int argc, char const *argv[])
{
	float x =23.1415;
	
	float y = do_math(x);

	printf("%f\n", y);
	
	return 0;
}
```
Display the relocations
```
$ readelf --relocs ./p

Relocation section '.rela.dyn' at offset 0x508 contains 1 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
000000600ff8  000500000006 R_X86_64_GLOB_DAT 0000000000000000 __gmon_start__ + 0

Relocation section '.rela.plt' at offset 0x520 contains 4 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
000000601018  000200000007 R_X86_64_JUMP_SLO 0000000000000000 do_math + 0
000000601020  000300000007 R_X86_64_JUMP_SLO 0000000000000000 printf + 0
000000601028  000400000007 R_X86_64_JUMP_SLO 0000000000000000 __libc_start_main + 0
000000601030  000500000007 R_X86_64_JUMP_SLO 0000000000000000 __gmon_start__ + 0

$ readelf --relocs ./libmymath.so 

Relocation section '.rela.dyn' at offset 0x4a0 contains 9 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
000000200df8  000000000008 R_X86_64_RELATIVE                    6d0
000000200e00  000000000008 R_X86_64_RELATIVE                    690
000000201030  000000000008 R_X86_64_RELATIVE                    201030
000000200fd0  000200000006 R_X86_64_GLOB_DAT 0000000000000000 _ITM_deregisterTMClone + 0
000000200fd8  000e00000006 R_X86_64_GLOB_DAT 0000000000201038 global_var + 0
000000200fe0  000300000006 R_X86_64_GLOB_DAT 0000000000000000 __gmon_start__ + 0
000000200fe8  000400000006 R_X86_64_GLOB_DAT 0000000000000000 _Jv_RegisterClasses + 0
000000200ff0  000500000006 R_X86_64_GLOB_DAT 0000000000000000 _ITM_registerTMCloneTa + 0
000000200ff8  000600000006 R_X86_64_GLOB_DAT 0000000000000000 __cxa_finalize + 0

Relocation section '.rela.plt' at offset 0x578 contains 3 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
000000201018  000900000007 R_X86_64_JUMP_SLO 0000000000000705 cubed + 0
000000201020  000300000007 R_X86_64_JUMP_SLO 0000000000000000 __gmon_start__ + 0
000000201028  000600000007 R_X86_64_JUMP_SLO 0000000000000000 __cxa_finalize + 0
```
disassemble
```
$ gdb ./p
GNU gdb (Ubuntu 7.7-0ubuntu3.1) 7.7
Copyright (C) 2014 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
and "show warranty" for details.
This GDB was configured as "x86_64-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<http://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
<http://www.gnu.org/software/gdb/documentation/>.
For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from ./p...done.
(gdb) disassemble main
Dump of assembler code for function main:
   0x00000000004006dd <+0>:	push   %rbp
   0x00000000004006de <+1>:	mov    %rsp,%rbp
   0x00000000004006e1 <+4>:	sub    $0x20,%rsp
   0x00000000004006e5 <+8>:	mov    %edi,-0x14(%rbp)
   0x00000000004006e8 <+11>:	mov    %rsi,-0x20(%rbp)
   0x00000000004006ec <+15>:	mov    0xc6(%rip),%eax        # 0x4007b8
   0x00000000004006f2 <+21>:	mov    %eax,-0x8(%rbp)
   0x00000000004006f5 <+24>:	mov    -0x8(%rbp),%eax
   0x00000000004006f8 <+27>:	mov    %eax,-0x18(%rbp)
   0x00000000004006fb <+30>:	movss  -0x18(%rbp),%xmm0
   0x0000000000400700 <+35>:	callq  0x4005b0 <do_math@plt>
   0x0000000000400705 <+40>:	movss  %xmm0,-0x18(%rbp)
   0x000000000040070a <+45>:	mov    -0x18(%rbp),%eax
   0x000000000040070d <+48>:	mov    %eax,-0x4(%rbp)
   0x0000000000400710 <+51>:	movss  -0x4(%rbp),%xmm0
   0x0000000000400715 <+56>:	cvtps2pd %xmm0,%xmm0
   0x0000000000400718 <+59>:	mov    $0x4007b4,%edi
   0x000000000040071d <+64>:	mov    $0x1,%eax
   0x0000000000400722 <+69>:	callq  0x4005c0 <printf@plt>
   0x0000000000400727 <+74>:	mov    $0x0,%eax
   0x000000000040072c <+79>:	leaveq 
   0x000000000040072d <+80>:	retq   
End of assembler dump.
(gdb) disassemble 0x4005b0
Dump of assembler code for function do_math@plt:
   0x00000000004005b0 <+0>:	jmpq   *0x200a62(%rip)        # 0x601018 <do_math@got.plt>
   0x00000000004005b6 <+6>:	pushq  $0x0
   0x00000000004005bb <+11>:	jmpq   0x4005a0
End of assembler dump.
(gdb) info symbol 0x601018
do_math@got.plt in section .got.plt
(gdb) x 0x601018
0x601018 <do_math@got.plt>:	0x004005b6
(gdb) disassemble 0x4005a0
No function contains specified address.
(gdb) x/i 0x4005a0
   0x4005a0:	pushq  0x200a62(%rip)        # 0x601008
(gdb) x/10i 0x4005a0
   0x4005a0:	pushq  0x200a62(%rip)        # 0x601008
   0x4005a6:	jmpq   *0x200a64(%rip)        # 0x601010
   0x4005ac:	nopl   0x0(%rax)
   0x4005b0 <do_math@plt>:	jmpq   *0x200a62(%rip)        # 0x601018 <do_math@got.plt>
   0x4005b6 <do_math@plt+6>:	pushq  $0x0
   0x4005bb <do_math@plt+11>:	jmpq   0x4005a0
   0x4005c0 <printf@plt>:	jmpq   *0x200a5a(%rip)        # 0x601020 <printf@got.plt>
   0x4005c6 <printf@plt+6>:	pushq  $0x1
   0x4005cb <printf@plt+11>:	jmpq   0x4005a0
   0x4005d0 <__libc_start_main@plt>:	jmpq   *0x200a52(%rip)        # 0x601028 <__libc_start_main@got.plt>
(gdb) break do_math
Breakpoint 1 at 0x4005b0
(gdb) run
Starting program: ./p 

Breakpoint 1, do_math (a=23.1415005) at mymath.c:10
10		float ret = cubed(a);
(gdb) x/x 0x601018
0x601018 <do_math@got.plt>:	0xf7bd872f

NOTE: format in examine is sticky, for example if you use the command x/c
      subsequent executions of x will use /c format. you therefore need to
      explicitly change the format to /d /c /s etc. for interpreting memory
      contents as differnt type from the previous call to x

(gdb) disassemble do_math
Dump of assembler code for function do_math:
   0x00007ffff7bd872f <+0>:	push   %rbp
   0x00007ffff7bd8730 <+1>:	mov    %rsp,%rbp
   0x00007ffff7bd8733 <+4>:	sub    $0x20,%rsp
   0x00007ffff7bd8737 <+8>:	movss  %xmm0,-0x14(%rbp)
=> 0x00007ffff7bd873c <+13>:	mov    -0x14(%rbp),%eax
   0x00007ffff7bd873f <+16>:	mov    %eax,-0x18(%rbp)
   0x00007ffff7bd8742 <+19>:	movss  -0x18(%rbp),%xmm0
   0x00007ffff7bd8747 <+24>:	callq  0x7ffff7bd85f0 <cubed@plt>
   0x00007ffff7bd874c <+29>:	movss  %xmm0,-0x18(%rbp)
   0x00007ffff7bd8751 <+34>:	mov    -0x18(%rbp),%eax
   0x00007ffff7bd8754 <+37>:	mov    %eax,-0x4(%rbp)
   0x00007ffff7bd8757 <+40>:	mov    0x20087a(%rip),%rax        # 0x7ffff7dd8fd8
   0x00007ffff7bd875e <+47>:	mov    (%rax),%eax
   0x00007ffff7bd8760 <+49>:	cvtsi2ss %eax,%xmm0
   0x00007ffff7bd8764 <+53>:	movss  -0x4(%rbp),%xmm1
   0x00007ffff7bd8769 <+58>:	addss  %xmm1,%xmm0
   0x00007ffff7bd876d <+62>:	movss  %xmm0,-0x4(%rbp)
   0x00007ffff7bd8772 <+67>:	mov    -0x4(%rbp),%eax
   0x00007ffff7bd8775 <+70>:	mov    %eax,-0x18(%rbp)
   0x00007ffff7bd8778 <+73>:	movss  -0x18(%rbp),%xmm0
   0x00007ffff7bd877d <+78>:	leaveq 
   0x00007ffff7bd877e <+79>:	retq   
End of assembler dump.
(gdb) disassemble 0x7ffff7bd85f0
Dump of assembler code for function cubed@plt:
   0x00007ffff7bd85f0 <+0>:	jmpq   *0x200a22(%rip)        # 0x7ffff7dd9018
   0x00007ffff7bd85f6 <+6>:	pushq  $0x0
   0x00007ffff7bd85fb <+11>:	jmpq   0x7ffff7bd85e0
End of assembler dump.
(gdb) info symbol 0x7ffff7dd9018
_GLOBAL_OFFSET_TABLE_ + 24 in section .got.plt of ./libmymath.so
```
## Interposition
main2.c
```
#include <stdio.h>

float do_math(float a);

float cubed(float a)
{
	return a*a*a*a;
}

int main(int argc, char const *argv[])
{
	float x =23.1415;
	
	float y = do_math(x);

	printf("%f\n", y);
	
	return 0;
}
```
compile and run
```
$ ./p2
286801.375000
```
disassemble
```
$ gdb ./p2
GNU gdb (Ubuntu 7.7-0ubuntu3.1) 7.7
Copyright (C) 2014 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
and "show warranty" for details.
This GDB was configured as "x86_64-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<http://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
<http://www.gnu.org/software/gdb/documentation/>.
For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from ./p2...done.
(gdb) disassemble do_math
Dump of assembler code for function do_math:
   0x00007ffff7bd872f <+0>:		push   %rbp
   0x00007ffff7bd8730 <+1>:		mov    %rsp,%rbp
   0x00007ffff7bd8733 <+4>:		sub    $0x20,%rsp
   0x00007ffff7bd8737 <+8>:		movss  %xmm0,-0x14(%rbp)
=> 0x00007ffff7bd873c <+13>:	mov    -0x14(%rbp),%eax
   0x00007ffff7bd873f <+16>:	mov    %eax,-0x18(%rbp)
   0x00007ffff7bd8742 <+19>:	movss  -0x18(%rbp),%xmm0
   0x00007ffff7bd8747 <+24>:	callq  0x7ffff7bd85f0 <cubed@plt>
   0x00007ffff7bd874c <+29>:	movss  %xmm0,-0x18(%rbp)
   0x00007ffff7bd8751 <+34>:	mov    -0x18(%rbp),%eax
   0x00007ffff7bd8754 <+37>:	mov    %eax,-0x4(%rbp)
   0x00007ffff7bd8757 <+40>:	mov    0x20087a(%rip),%rax        # 0x7ffff7dd8fd8
   0x00007ffff7bd875e <+47>:	mov    (%rax),%eax
   0x00007ffff7bd8760 <+49>:	cvtsi2ss %eax,%xmm0
   0x00007ffff7bd8764 <+53>:	movss  -0x4(%rbp),%xmm1
   0x00007ffff7bd8769 <+58>:	addss  %xmm1,%xmm0
   0x00007ffff7bd876d <+62>:	movss  %xmm0,-0x4(%rbp)
   0x00007ffff7bd8772 <+67>:	mov    -0x4(%rbp),%eax
   0x00007ffff7bd8775 <+70>:	mov    %eax,-0x18(%rbp)
   0x00007ffff7bd8778 <+73>:	movss  -0x18(%rbp),%xmm0
   0x00007ffff7bd877d <+78>:	leaveq 
   0x00007ffff7bd877e <+79>:	retq   
End of assembler dump.
```
call chain `_dl_runtime_resolve() -> _dl_fixup() -> _dl_lookup_symbol_x()`
```
(gdb) disassemble _dl_runtime_resolve
Dump of assembler code for function _dl_runtime_resolve:
   0x00007ffff7df04e0 <+0>:		sub    $0x38,%rsp
   0x00007ffff7df04e4 <+4>:		mov    %rax,(%rsp)
   0x00007ffff7df04e8 <+8>:		mov    %rcx,0x8(%rsp)
   0x00007ffff7df04ed <+13>:	mov    %rdx,0x10(%rsp)
   0x00007ffff7df04f2 <+18>:	mov    %rsi,0x18(%rsp)
   0x00007ffff7df04f7 <+23>:	mov    %rdi,0x20(%rsp)
   0x00007ffff7df04fc <+28>:	mov    %r8,0x28(%rsp)
   0x00007ffff7df0501 <+33>:	mov    %r9,0x30(%rsp)
   0x00007ffff7df0506 <+38>:	mov    0x40(%rsp),%rsi
   0x00007ffff7df050b <+43>:	mov    0x38(%rsp),%rdi
   0x00007ffff7df0510 <+48>:	callq  0x7ffff7de9460 <_dl_fixup>
   0x00007ffff7df0515 <+53>:	mov    %rax,%r11
   0x00007ffff7df0518 <+56>:	mov    0x30(%rsp),%r9
   0x00007ffff7df051d <+61>:	mov    0x28(%rsp),%r8
   0x00007ffff7df0522 <+66>:	mov    0x20(%rsp),%rdi
   0x00007ffff7df0527 <+71>:	mov    0x18(%rsp),%rsi
   0x00007ffff7df052c <+76>:	mov    0x10(%rsp),%rdx
   0x00007ffff7df0531 <+81>:	mov    0x8(%rsp),%rcx
   0x00007ffff7df0536 <+86>:	mov    (%rsp),%rax
   0x00007ffff7df053a <+90>:	add    $0x48,%rsp
   0x00007ffff7df053e <+94>:	jmpq   *%r11
End of assembler dump.


_dl_lookup_symbol_x
/* Search loaded objects' symbol tables for a definition of the symbol UNDEF_NAME */
```
LD_DEBUG
```
$ LD_DEBUG=bindings,symbols ./p
4038:	transferring control: ./p
4038:	
4038:	symbol=do_math;  lookup in file=./p [0]
4038:	symbol=do_math;  lookup in file=./libmymath.so [0]
4038:	binding file ./p [0] to ./libmymath.so [0]: normal symbol `do_math'
4038:	symbol=cubed;  lookup in file=./p [0]
4038:	symbol=cubed;  lookup in file=./libmymath.so [0]
4038:	binding file ./libmymath.so [0] to ./libmymath.so [0]: normal symbol `cubed'
4038:	symbol=printf;  lookup in file=./p [0]
4038:	symbol=printf;  lookup in file=./libmymath.so [0]
4038:	symbol=printf;  lookup in file=/lib/x86_64-linux-gnu/libc.so.6 [0]
4038:	binding file ./p [0] to /lib/x86_64-linux-gnu/libc.so.6 [0]: normal symbol `printf' [GLIBC_2.2.5]

$ LD_DEBUG=bindings,symbols ./p2
4042:	transferring control: ./p2
4042:	
4042:	symbol=do_math;  lookup in file=./p2 [0]
4042:	symbol=do_math;  lookup in file=./libmymath.so [0]
4042:	binding file ./p2 [0] to ./libmymath.so [0]: normal symbol `do_math'
4042:	symbol=cubed;  lookup in file=./p2 [0]
4042:	binding file ./libmymath.so [0] to ./p2 [0]: normal symbol `cubed'
4042:	symbol=printf;  lookup in file=./p2 [0]
4042:	symbol=printf;  lookup in file=./libmymath.so [0]
4042:	symbol=printf;  lookup in file=/lib/x86_64-linux-gnu/libc.so.6 [0]
4042:	binding file ./p2 [0] to /lib/x86_64-linux-gnu/libc.so.6 [0]: normal symbol `printf' [GLIBC_2.2.5]
```
-Bsymbolic-functions
```
$ readelf --relocs ./libmymath.so 

Relocation section '.rela.dyn' at offset 0x4a0 contains 9 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
000000200df8  000000000008 R_X86_64_RELATIVE                    6b0
000000200e00  000000000008 R_X86_64_RELATIVE                    670
000000201028  000000000008 R_X86_64_RELATIVE                    201028
000000200fd0  000200000006 R_X86_64_GLOB_DAT 0000000000000000 _ITM_deregisterTMClone + 0
000000200fd8  000e00000006 R_X86_64_GLOB_DAT 0000000000201030 global_var + 0
000000200fe0  000300000006 R_X86_64_GLOB_DAT 0000000000000000 __gmon_start__ + 0
000000200fe8  000400000006 R_X86_64_GLOB_DAT 0000000000000000 _Jv_RegisterClasses + 0
000000200ff0  000500000006 R_X86_64_GLOB_DAT 0000000000000000 _ITM_registerTMCloneTa + 0
000000200ff8  000600000006 R_X86_64_GLOB_DAT 0000000000000000 __cxa_finalize + 0

Relocation section '.rela.plt' at offset 0x578 contains 2 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
000000201018  000300000007 R_X86_64_JUMP_SLO 0000000000000000 __gmon_start__ + 0
000000201020  000600000007 R_X86_64_JUMP_SLO 0000000000000000 __cxa_finalize + 0
```
symbol table
```
$ readelf --symbols ./p2

Symbol table '.dynsym' contains 14 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_deregisterTMCloneTab
     2: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND do_math
     3: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND printf@GLIBC_2.2.5 (2)
     4: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __libc_start_main@GLIBC_2.2.5 (2)
     5: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND __gmon_start__
     6: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _Jv_RegisterClasses
     7: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_registerTMCloneTable
     8: 0000000000601048     0 NOTYPE  GLOBAL DEFAULT   24 _edata
     9: 000000000040070d    47 FUNC    GLOBAL DEFAULT   13 cubed
    10: 0000000000601050     0 NOTYPE  GLOBAL DEFAULT   25 _end
    11: 0000000000601048     0 NOTYPE  GLOBAL DEFAULT   25 __bss_start
    12: 00000000004005a8     0 FUNC    GLOBAL DEFAULT   11 _init
    13: 0000000000400804     0 FUNC    GLOBAL DEFAULT   14 _fini

Symbol table '.symtab' contains 72 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000400238     0 SECTION LOCAL  DEFAULT    1 
     2: 0000000000400254     0 SECTION LOCAL  DEFAULT    2 
     3: 0000000000400274     0 SECTION LOCAL  DEFAULT    3 
     4: 0000000000400298     0 SECTION LOCAL  DEFAULT    4 
     5: 00000000004002d8     0 SECTION LOCAL  DEFAULT    5 
     6: 0000000000400428     0 SECTION LOCAL  DEFAULT    6 
     7: 00000000004004f2     0 SECTION LOCAL  DEFAULT    7 
     8: 0000000000400510     0 SECTION LOCAL  DEFAULT    8 
     9: 0000000000400530     0 SECTION LOCAL  DEFAULT    9 
    10: 0000000000400548     0 SECTION LOCAL  DEFAULT   10 
    11: 00000000004005a8     0 SECTION LOCAL  DEFAULT   11 
    12: 00000000004005d0     0 SECTION LOCAL  DEFAULT   12 
    13: 0000000000400620     0 SECTION LOCAL  DEFAULT   13 
    14: 0000000000400804     0 SECTION LOCAL  DEFAULT   14 
    15: 0000000000400810     0 SECTION LOCAL  DEFAULT   15 
    16: 000000000040081c     0 SECTION LOCAL  DEFAULT   16 
    17: 0000000000400858     0 SECTION LOCAL  DEFAULT   17 
    18: 0000000000600e00     0 SECTION LOCAL  DEFAULT   18 
    19: 0000000000600e08     0 SECTION LOCAL  DEFAULT   19 
    20: 0000000000600e10     0 SECTION LOCAL  DEFAULT   20 
    21: 0000000000600e18     0 SECTION LOCAL  DEFAULT   21 
    22: 0000000000600ff8     0 SECTION LOCAL  DEFAULT   22 
    23: 0000000000601000     0 SECTION LOCAL  DEFAULT   23 
    24: 0000000000601038     0 SECTION LOCAL  DEFAULT   24 
    25: 0000000000601048     0 SECTION LOCAL  DEFAULT   25 
    26: 0000000000000000     0 SECTION LOCAL  DEFAULT   26 
    27: 0000000000000000     0 SECTION LOCAL  DEFAULT   27 
    28: 0000000000000000     0 SECTION LOCAL  DEFAULT   28 
    29: 0000000000000000     0 SECTION LOCAL  DEFAULT   29 
    30: 0000000000000000     0 SECTION LOCAL  DEFAULT   30 
    31: 0000000000000000     0 SECTION LOCAL  DEFAULT   31 
    32: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS crtstuff.c
    33: 0000000000600e10     0 OBJECT  LOCAL  DEFAULT   20 __JCR_LIST__
    34: 0000000000400650     0 FUNC    LOCAL  DEFAULT   13 deregister_tm_clones
    35: 0000000000400680     0 FUNC    LOCAL  DEFAULT   13 register_tm_clones
    36: 00000000004006c0     0 FUNC    LOCAL  DEFAULT   13 __do_global_dtors_aux
    37: 0000000000601048     1 OBJECT  LOCAL  DEFAULT   25 completed.6972
    38: 0000000000600e08     0 OBJECT  LOCAL  DEFAULT   19 __do_global_dtors_aux_fin
    39: 00000000004006e0     0 FUNC    LOCAL  DEFAULT   13 frame_dummy
    40: 0000000000600e00     0 OBJECT  LOCAL  DEFAULT   18 __frame_dummy_init_array_
    41: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS main2.c
    42: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS crtstuff.c
    43: 0000000000400968     0 OBJECT  LOCAL  DEFAULT   17 __FRAME_END__
    44: 0000000000600e10     0 OBJECT  LOCAL  DEFAULT   20 __JCR_END__
    45: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS 
    46: 0000000000600e08     0 NOTYPE  LOCAL  DEFAULT   18 __init_array_end
    47: 0000000000600e18     0 OBJECT  LOCAL  DEFAULT   21 _DYNAMIC
    48: 0000000000600e00     0 NOTYPE  LOCAL  DEFAULT   18 __init_array_start
    49: 0000000000601000     0 OBJECT  LOCAL  DEFAULT   23 _GLOBAL_OFFSET_TABLE_
    50: 0000000000400800     2 FUNC    GLOBAL DEFAULT   13 __libc_csu_fini
    51: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_deregisterTMCloneTab
    52: 0000000000601038     0 NOTYPE  WEAK   DEFAULT   24 data_start
    53: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND do_math
    54: 0000000000601048     0 NOTYPE  GLOBAL DEFAULT   24 _edata
    55: 000000000040070d    47 FUNC    GLOBAL DEFAULT   13 cubed
    56: 0000000000400804     0 FUNC    GLOBAL DEFAULT   14 _fini
    57: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND printf@@GLIBC_2.2.5
    58: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __libc_start_main@@GLIBC_
    59: 0000000000601038     0 NOTYPE  GLOBAL DEFAULT   24 __data_start
    60: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND __gmon_start__
    61: 0000000000601040     0 OBJECT  GLOBAL HIDDEN    24 __dso_handle
    62: 0000000000400810     4 OBJECT  GLOBAL DEFAULT   15 _IO_stdin_used
    63: 0000000000400790   101 FUNC    GLOBAL DEFAULT   13 __libc_csu_init
    64: 0000000000601050     0 NOTYPE  GLOBAL DEFAULT   25 _end
    65: 0000000000400620     0 FUNC    GLOBAL DEFAULT   13 _start
    66: 0000000000601048     0 NOTYPE  GLOBAL DEFAULT   25 __bss_start
    67: 000000000040073c    81 FUNC    GLOBAL DEFAULT   13 main
    68: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _Jv_RegisterClasses
    69: 0000000000601048     0 OBJECT  GLOBAL HIDDEN    24 __TMC_END__
    70: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_registerTMCloneTable
    71: 00000000004005a8     0 FUNC    GLOBAL DEFAULT   11 _init
```
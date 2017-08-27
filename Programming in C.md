# Programming in C
## Working with Functions
### Automatic and Static Variables
If you place the word `static` in front of a variable declaration, you are in an entirely new ballgame. The word `static` in C refers to an electric charge, but rather to the notion of something that has no movement. This is the key to the concept of a static variable - it does *not* come and go as the function is called and returns. This implies that the value a static variable has upon leaving a function is the same value that variable will have the next time the function is called.

Static variables also differ with respect to their initialization. A static, local variable is initialized *only* once at the start of overall program execution - and not each time that the function is called.

In the function `auto_static()`, which is defined as follows:
```C
void auto_static (void)
{
    static int staticVar = 100;
             .
             .
             .
}
```
the value of `staticVar` is initialized to 100 only once when program execution begins.


```C
#include <stdio.h>

void auto_static (void)
{
    int autoVar = 1;
    static int staticVar = 1;

    printf("automatic = %i. static = %i\n", autoVar, staticVar);

    ++staticVar;
    ++autoVar;
}

int main (void)
{
    int i;
    void auto_static (void);

    for ( i = 0; i < 5; ++i)
          auto_static();

    return 0;
}
```
```
automatic = 1, static = 1
automatic = 1, static = 2
automatic = 1, static = 3
automatic = 1, static = 4
automatic = 1, static = 5
```
Inside the `auto_static()` function, two local variables are declared. The first variable, called `autoVar`, is an `automatic` variable of type `int` with an initial value of `1`. The second variable, called `staticVar`, is a static variable, also of type `int` and also with an initial value of `1`. The function calls the `printf()` routine to display the values of these two variables. After this, the variables are each incremented by `1`, and execution of the function is then complete.

The `main()` routine sets up a loop to call the `auto_static()` function five times. The output points out the difference between the two variable types. The value of the `automatic` variable is listed as `1` for each line of the display. This is because its value is set to `1` each time the function is called. On the other hand, the output shows the value of the static variable steadily increasing from `1` through `5`. This is because its value is set equal to 1 only once - when program execution begins - and because its value is retained from one function call to the next.

## Working with Larger Programs
### Communication Between Modules
Several methods can be used so that the modules contained in separate files can effectively communicate. If a function from one file needs to call a function contained inside another file, the function call can be made in the normal fashion, and arguments can be passed and returned in the usual way. Of course, in the file that calls the function, you should *always make certain to include a prototype declaration so the compiler knows the function's argument types and the type of the return value*.

#### External Variables
Functions contained in separate files can communicate through external *variables*, which are effectively an extension to the concept of the global variable.

An external variable is one whose value can be accessed and changed by another module. Inside the module that wants to access the external variable, the variable is declared in the normal fashion and the keyword `extern` is placed before the declaration. This signals to the system that a globally defined variable from another file is to be accessed.

Suppose you want to define an `int` variable called `moveNumber`, whose value you want to access and possibly modify from within a function contained in another file. In Chapter 7, you learned that if you wrote the statement
```C
int moveNumber = 0;
```
at the beginning of your program, *outside* of nay function, then its value could be referenced by any function within that program. In such a case, `moveNumber` was defined as a global variable.

Actually, this same definition of the variable `moveNumber` also makes its value accessible by functions contained in other files. Specifically, the preceding statement defines the variable `moveNumber` not just a global variable, but also an *external* global variable. To reference the value of an external global variable from another module, you must declare the variable to be accessed, preceding the declaration with the keyword `extern`, as follows:
```C
extern int moveNumber;
```
The value of `moveNumber` can now be accessed and modified by the module in which the preceding declaration appears. Other modules can also access the value of `moveNumber` by incorporating a similar `extern` declaration in the file.

You must obey an important rule when working with external variables. The variable has to be *defined* in some place among your source files. This is done in one of two ways. The first way is to declare the variable outside of any function, *not* preceded by the keyword `extern`, as follows:
```
int moveNumber;
```
Here, an initial value can be optionally assigned to the variable, as shown previously.

#### `Static` Versus `Extern` Variables and Functions
You now know that any variable defined outside of a function is not only a global variable, but is also an external variable. Many situations arise in which you want to define a variable to be global but not *external*. In other words, you want to define a global variable to be local to a particular module (file). It makes sense to want to define a variable this way if no functions other than those contained inside a particular file need access to the particular variable. This can be accomplished in C by defining the variable to be `static`. The statement
```C
static int moveNumber = 0;
```
if made outside of any function, makes the value of `moveNumber` accessible from any subsequent point in the file in which the definition appears, *but not from functions contained in other files*.

As mentioned earlier in this chapter, you can directly call a function defined in another file. Unlike variables, no special mechanisms are required; that is, to call a function contained in another file, you don't need an `extern` declaration for that function.

When a function is *defined*, it can be declared to be *extern* or static, the former case being the default. A static function can be called only from within the same file as the function appears. So, if you have a function called `squareRoot`, placing the keyword *static* before the function header declaration for this function makes it callable only from within the file in which it is defined.

Figure 14.1 summarizes communication between different modules. Here two modules are depicted, `mod1.c` and `mod2.c`.
```C
double x;                      |
static double result;          |
                               |
static void doSquare (void)    |
{                              |
    double square (void);      |
                               |
    x = 2.0;                   |
    result = square();         |
}                              |
                               |    
int main (void)                |
{                              |    extern double x;
    doSquare();                |    
    printf("%g\n", result);    |    double square(void)
                               |    {    
    return 0;                  |        return x * x;
}                              |    }
```
`mod1.c` defines two functions: `doSquare()` and `main()`. The way things are set up here, `main()` calls `doSquare()`, which in turn calls `square()`. This last function is defined in the module `mod2.c`.

Because `doSquare()` is declared `static`, it can only be called from within `mod1.c`, and by no other module. `mod1.c` defines two global variables: `x` and `result`, both of type `double`. `x` can be accessed by any module that is linked together with `mod1.c`. On the other hand, the keyword `static` in front of the definition of `result` means that it can only be accessed by functions defined inside `mod1.c` (namely `main()` and `doSquare()`).

When execution begins, the `main()` routine calls `doSquare()`. This function assigns the value `2.0` to the global variable `x` and then calls the function `square()`. Because `square()` is defined in another source file (inside `mod2.c`), and because it doesn't return an `int`, `doSquare()` properly includes an appropriate declaration at the beginning of the function.

The `square()` function returns as its value the square of the value of the global variable `x`. Because `square` wants to access the value of this variable, which is defined in another source file (in `mod1.c`), an appropriate `extern` declaration appears in `mod2.c` (and, in this case, it makes no difference whether the declaration occurs inside or outside the `square()` function).

#### Using Header Files Effectively
In Chapter, "The Preprocessor," you were introduced to the concept of the include file. As stated there, you can group all your commonly used definitions inside such a file and then simply include the file in any program that needs to use those definitions. Nowhere is the usefulness of the `#include` facility grater than in developing programs that have been divided into separate program modules.

If more than one program is working on developing a particular program, include files provide a means of standardization: Each programmer is using the same definitions, which have the same values. Furthermore, each programmer is thus spared the time-consuming and error-prone task of typing these definitions into each file that must use them.

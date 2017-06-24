## Modules
If you quit from the Python interpreter and enter it again, the definitions you have made (functions and variables) are lost. Therefore, if you want to write a somewhat longer program, you are better off using a text editor to prepare the input for the interpreter and running it with that file as input instead. This is known as creating a *script*. As your program gets longer, you may want to split it into several files for easier maintenance. You may also want to use a handy function that you've written in several programs without copying its definition into each program.

To support this, Python has a way to put definitions in a file and use them in a script or in an interactive instance of the interpreter. Such a file is called a *module*; definitions from a module can be *imported* into other modules or into the *main* module (the collection of variables that you have access to in a script executed at the top level and in calculator mode).

A module is a file containing Python definitions and statements. The file name is the module name with the suffix `.py` appended. For instance, use your favorite text editor to create a file called `fibo.py` in the current directory with the following contents:
```Python
# Fibonacci numbers module

def fib(n):    # write Fibonacci series up to n
    a, b = 0, 1
    while b < n:
        print b,
        a, b = b, a+b

def fib2(n):   # return Fibonacci series up to n
    result = []
    a, b = 0, 1
    while b < n:
        result.append(b)
        a, b = b, a+b
    return result
```
Now enter the Python interpreter and import this module with the following command:
```
>>> import fibo
```
Using the module name you can access the functions:
```
>>> fibo.fib(1000)
1 1 2 3 5 8 13 21 34 55 89 144 233 377 610 987
>>> fibo.fib2(100)
[1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89]
```
## Classes
### Defining a Class
- A *class* is a special data type which defines how to build a certain kind of object.
- *Instances* are objects that are created which follow the definition given inside of the class
### A simple class def: *student*
```Python
class student:
 """A class representing a student"""
 def __init__(self,n,a):
     self.full_name = n
     self.age = a
 def get_age(self):
     return self.age
```
### Instantiating Objects
- There is no "new" keyword as in Java.
- Just use the class name with () notation and assign the result to a variable
- `__init__` servers as a constructor for the class.
- The arguments passed to the class name are given to its `__init__` method
- So, the \_\_init\_\_ method for student is passed "Bob" and 21 and the new class instance is bound to b:
   ```
   b = student("Bob", 21)
   ```

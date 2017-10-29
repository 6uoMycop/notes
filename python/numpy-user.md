## Quickstart tutorial

### The Basics
NumPy's main object is the homogeneous multidimensional array. It is a table of elements (usually numbers), all of
the same type, indexed by a tuple of positive integers. In NumPy dimensions are called *axes*. The number of axes is
*rank*.

In the example pictured below, the array has rank 2 (it is 2-dimensional). The first dimension (axis) has a length of 2, the second dimension has a length of 3.
```
[[1., 0., 0.],
 [0., 1., 2.]]
```
#### Basic Operations
Arithmetic operators on arrays apply *elementwise*. A new array is created and filled with the result.
```
>>> a = np.array([20,30,40,50])
>>> b = np.arange(4)
>>> b
array([0, 1, 2, 3])
>>> c = a-b
>>> c
array([20, 29, 38, 47])
>>> b**2
array([0, 1, 4, 9])
```
Unlike in many matrix languages, the product operator `*` operates elementwise in NumPy arrays. The matrix product can be performed using the `dot` function or method:
```
>>> A = np.array([[1,1],
...             [0,1]])
>>> B = np.array([[2,0],
...             [3,4]])
>>> A*B                                # elementwise product
array([[2, 0],
       [0, 4]])
>>> A.dot(B)                           # matrix product
array([[5, 4],
       [3, 4]])
>>> np.dot(A, B)                       # another matrix product
array([[5, 4],
       [3, 4]])
```
By default, these operations apply to the array as though it were a list of numbers, regardless of its shape. However,
by specifying the `axis` parameter you can apply an operation along the specified axis of an array:
```
>>> b = np.arange(12).reshape(3,4)
>>> b
array([[ 0,  1,  2,  3],
       [ 4,  5,  6,  7],
       [ 8,  9, 10, 11]])
>>>
>>> b.sum(axis=0)                      # sum of each column
array([12, 15, 18, 21])
>>> b.min(axis=1)                      # sum of each row
array([0, 4, 8])
````
#### Indexing, Slicing and Iterating
**Multidimensional** arrays can have one index per axis. These indices are given in a tuple separated by commas:
```
>>> def f(x,y):
...     return 10*x+y
... 
>>> b = np.fromfunction(f,(5,4),dtype=int)
>>> b
array([[ 0,  1,  2,  3],
       [10, 11, 12, 13],
       [20, 21, 22, 23],
       [30, 31, 32, 33],
       [40, 41, 42, 43]])
>>> b[2,3]
23
>>> b[0:5, 1]                          # each row in the second column of b
array([ 1, 11, 21, 31, 41])
>>> b[ : ,1]                           # equivalent to the previous example
array([ 1, 11, 21, 31, 41])
>>> b[1:3, : ]                         # each column in the second and third row of b
array([[10, 11, 12, 13],
       [20, 21, 22, 23]])
```
When fewer indices are provided than the number of axes, the missing indices are considered complete slices:
```
>>> b[-1]                              # the last row. Equivalent to b[-1,:]
array([40, 41, 42, 43])
```
**Iterating** over multidimensional arrays is done with respect to the first axis:
```
>>> for row in b:
...     print(row)
... 
[0 1 2 3]
[10 11 12 13]
[20 21 22 23]
[30 31 32 33]
[40 41 42 43]
```

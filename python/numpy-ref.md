## Array objects
### The N-dimensional array (`ndarray`)
#### Matrix objects
matrix objects inherit from the ndarray and therefore, they have the same attributes and methods of ndarrays.

- Matrix objects are always two-dimensional.

```Python
class numpy.matrix
    """
    Returns a matrix from an array-like object, or from a string of data.

    Parameters
    ----------
    data : array_like or string

    Examples
    --------
    >>> np.matrix([[1, 2], [3, 4]])
    matrix([[1, 2],
            [3, 4]])

    """
```

```Python
matrix.tolist()
    """
    Return the matrix as a (possibly nested) list.

    Examples
    --------
    >>> x = np.matrix(np.arange(12).reshape((3,4))); x
    matrix([[ 0, 1, 2, 3],
            [ 4, 5, 6, 7],
            [ 8, 9, 10, 11]])
    >>> x.tolist()
    [[0, 1, 2, 3], [4, 5, 6, 7], [8, 9, 10, 11]]
    
    """
```

## Routines
### Array creation routines
#### Ones and zeros

```Python
    """
    Examples
    --------
    >>> np.zeros(5)
    array([ 0.,  0.,  0.,  0.,  0.])
    >>> np.zeros((2, 1))
    array([[ 0.],
           [ 0.]])
    >>> s = (2,2)
    >>> np.zeros(s)
    array([[ 0.,  0.],
           [ 0.,  0.]])
    """
```

```Python
def full(shape, fill_value, dtype=None, order='C'):
    """
    Return a new array of given shape and type, filled with `fill_value`.

    Examples
    --------
    >>> np.full((2, 2), np.inf)
    array([[ inf,  inf],
           [ inf,  inf]])
    >>> np.full((2, 2), 10)
    array([[10, 10],
           [10, 10]])

    """
```

#### From existing data

```Python
def loadtxt(fname, dtype=float, comments='#', delimiter=None,
            converters=None, skiprows=0, usecols=None, unpack=False,
            ndmin=0):
    """
    Load data from a text file.
    
    Parameters
    ----------
    dtype : data-type, optional
        Data-type of the resulting array; default: float.
    delimiter : str, optional
        The string used to separate values.  By default, this is any
        whitespace.
    
    usecols : int or sequence, optional
        Which columns to read, with 0 being the first.

    Returns
    -------
    out : ndarray
        Data read from the text file.

    """
```

### Array manipulation routines
#### Joining arrays

```Python
numpy.concatenate((a1, a2, ...), axis=0)
    """
    Join a sequence of arrays along an existing axis.

    Parameters
    ----------
    a1, a2, ... : sequence of array_like
        The arrays must have the same shape, except in the dimension
        corresponding to axis (the first, by default).
    axis : int, optional
        The axis along which the arrays will be joined. Default is 0.

    Examples
    --------
    >>> a = np.array([[1, 2], [3, 4]])
    >>> b = np.array([[5, 6]])
    >>> np.concatenate((a, b), axis=0)
    array([[1, 2],
           [3, 4],
           [5, 6]])
    >>> np.concatenate((a, b.T), axis=1)
    array([[1, 2, 5],
           [3, 4, 6]])

    """
```

```Python
def vstack(tup):
    """
    Stack arrays in sequence vertically (row wise).

    Take a sequence of arrays and stack them vertically to make a single
    array.

    Notes
    -----
    Equivalent to ``np.concatenate(tup, axis=0)`` if `tup` contains arrays that
    are at least 2-dimensional.

    Examples
    --------
    >>> a = np.array([1, 2, 3])
    >>> b = np.array([2, 3, 4])
    >>> np.vstack((a,b))
    array([[1, 2, 3],
           [2, 3, 4]])

    """
```

#### Adding and removing elements
```Python
def unique(ar, return_index=False, return_inverse=False,
           return_counts=False, axis=None):
    """
    Find the unique elements of an array.

    Returns the sorted unique elements of an array. There are three optional
    outputs in addition to the unique elements: the indices of the input array
    that give the unique values, the indices of the unique array that
    reconstruct the input array, and the number of times each unique value
    comes up in the input array.

    Parameters
    ----------
    return_counts : bool, optional
        If True, also return the number of times each unique item appears
        in `ar`.
        .. versionadded:: 1.9.0



    Returns
    -------
    unique : ndarray
        The sorted unique values.
    unique_counts : ndarray, optional
        The number of times each of the unique values comes up in the
        original array. Only provided if `return_counts` is True.
        .. versionadded:: 1.9.0

    """
```

### Input and output
#### Text formatting options

```Python
numpy.set_printoptions(precision=None)
    """
    Set printing options.
    
    Parameters
    ----------
    precision : int, optional
        Number of digits of precision for floating point output (default 8).

    """
```

### Linear algebra (`numpy.linalg`)
#### Decompositions
```Python
def svd(a, full_matrices=1, compute_uv=1):
    """
    Singular Value Decomposition.

    Factors the matrix `a` as ``u * np.diag(s) * v``, where `u` and `v`
    are unitary and `s` is a 1-d array of `a`'s singular values.
    
    Parameters
    ----------
    a : (..., M, N) array_like
        A real or complex matrix of shape (`M`, `N`) .
    full_matrices : bool, optional
        If True (default), `u` and `v` have the shapes (`M`, `M`) and
        (`N`, `N`), respectively.  Otherwise, the shapes are (`M`, `K`)
        and (`K`, `N`), respectively, where `K` = min(`M`, `N`).
    compute_uv : bool, optional
        Whether or not to compute `u` and `v` in addition to `s`.  True
        by default.
    
    Returns
    -------
    u : { (..., M, M), (..., M, K) } array
        Unitary matrices. The actual shape depends on the value of
        ``full_matrices``. Only returned when ``compute_uv`` is True.
    s : (..., K) array
        The singular values for every matrix, sorted in descending order.
    v : { (..., N, N), (..., K, N) } array
        Unitary matrices. The actual shape depends on the value of
        ``full_matrices``. Only returned when ``compute_uv`` is True.
    
    Notes
    -----
    
    The SVD is commonly written as ``a = U S V.H``.  The `v` returned
    by this function is ``V.H`` and ``u = U``.

    If ``U`` is a unitary matrix, it means that it
    satisfies ``U.H = inv(U)``.

    Examples
    --------
    >>> a = np.random.randn(9, 6) + 1j*np.random.randn(9, 6)

    Reconstruction based on reduced SVD:

    >>> U, s, V = np.linalg.svd(a, full_matrices=False)
    >>> U.shape, V.shape, s.shape
    ((9, 6), (6, 6), (6,))
    >>> S = np.diag(s)
    >>> np.allclose(a, np.dot(U, np.dot(S, V)))
    True

    """
```

### Statistics
#### Order statistics
```Python
def amax(a, axis=None, out=None, keepdims=np._NoValue):
    """
    Return the maximum of an array or maximum along an axis.

    Examples
    --------
    >>> a = np.arange(4).reshape((2,2))
    >>> a
    array([[0, 1],
           [2, 3]])
    >>> np.amax(a)           # Maximum of the flattened array
    3

    """
```

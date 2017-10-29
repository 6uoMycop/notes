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

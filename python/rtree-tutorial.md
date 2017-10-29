## Creating an index
### Import
After installing Rtree, you should be able to open up a Python prompt and issue the following:
```
>>> from rtree import index
```

### Construct an instance
After importing the index module, construct an index with the default construction:
```
>>> idx = index.Index()
```

### Create a bounding box

> Note: The coordinate ordering for all functions are sensitive the the index's **interleaved** data member. If **interleaved** is False, the coordinates must be in the form [xmin, xmax, ymin, ymax, ..., ..., kmin, kmax]. If **interleaved** is True, the coordinates must be in the form [xmin, ymin, ..., kmin, xmax, ymax, ..., kmax].

### Insert records into the index
Insert an entry into the index:
```
>>> idx.insert(0, (left, bottom, right, top))
```

> Note: Inserting a point, i.e. where left == right && top == bottom, will essentially insert a single point entry into the index instead of copying extra coordinates and inserting them. There is no shortcut to explicitly insert a single point, however.

### Query the index
There are three primary methods for querying the index. **rtree.index.Index.intersection()** will return you index entries that cross or are contained within the given query window. **rtree.index.Index.intersection()**

#### Intersection
Given a query window, return ids that are contained within the window:
```
>>> list(idx.intersection((1.0, 1.0, 2.0, 2.0)))
[0]
```
Given a query window that is beyond the bounds of data we have in the index:
```
>>> list(idx.intersection((1.0000001, 1.0000001, 2.0, 2.0)))
[]
```
#### Nearest Neighbors
The following finds the 1 nearest item to the given bounds. If multiple items are of equal distance to the bounds, both are returned:
```
>>> idx.insert(1, (left, bottom, right, top))
>>> list(idx.nearest((1.0000001, 1.0000001, 2.0, 2.0), 1))
[0, 1]
```

> Example of finding the three items nearest to this one:
> ```Python
> >>> from rtree import index
> >>> idx = index.Index()
> >>> idx.insert(4321, (34.37, 26.73, 49.37, 41.73), obj=42)
> >>> hits = idx.nearest((0, 0, 10, 10), 3, objects=True)
> ```

## 3D indexes
As of Rtree version 0.5.0, you can create 3D (actually kD) indexes. The following is a 3D index that is to be stored on disk. Persisted indexes are stored on disk using two files – an index file (.idx) and a data (.dat) file. You can modify the extensions these files use by altering the properties of the index at instantiation time. The following creates a 3D index that is stored on disk as the files `3d_index.data` and `3d_index.index`:
```
>>> from rtree import index
>>> p = index.Property()
>>> p.dimension = 3
>>> p.dat_extension = 'data'
>>> p.idx_extension = 'index'
>>> idx3d = index.Index('3d_index',properties=p)
>>> idx3d.insert(1, (0, 0, 60, 60, 23.0, 42.0))
>>> idx3d.intersection( (-1, -1, 62, 62, 22, 43))
[1L]
```

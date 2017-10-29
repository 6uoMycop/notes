First, download and install version 1.7.0 of the libspatialindex library from:

http://libspatialindex.github.com

The library is a GNU-style build, so it is a matter of:
```
$ ./configure; make; make install
```
You may need to run the `ldconfig` command after installing the library to ensure that applications can find it at startup time.

At this point you can get Rtree 0.7.0 via easy_install:
```
$ easy_install Rtree
```
or by running the local setup.py:
```
$ python setup.py install
```
You can build and test in place like:
```
$ python setup.py test
```

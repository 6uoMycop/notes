### Class Loader
class files are not loaded into memory all at once, but rather are loaded on demand, as needed by the program. The class loader is the part of the JVM that loads classes into memory.

By default, Java application typically has the following class loaders:
- Bootstrap class loader
  - Loads internal classes (e.g. the application class loader)
  - Loads basic classes in package java.*
  - Path can be set by JVM options
    - sets the path, `-Xbootclasspath:<directories and/or jars>`
    - appends to the path, `-Xbootclasspath/a:<directories and/or jars>`
    - prepends to the path, `-Xbootclasspath/p:<directories and/or jars>`
- Extension class loader
  - loads extension classes
  - Path can be set by JVM option `-Djava.ext.dirs=<directory/directories>`
- System (Application) Class Loader
  - Loads the application's classes
  - Path can be set by JVM option
    - `-cp <directories and or jars>`
    - `-classpath <directories and or jars>`
    - `-Djava.class.path=<directories and or jars>`
    - the `CLASSPATH` environment variable

In addition, an application may install one or more user-defined (custom) class loaders, e.g. `MyClassLoader`.

By convention, when a class loader is asked to load a class, it first asks the class that loaded it to load that class. I.e. the following delegation model exists:
```
-------------      -----------------       -----------------       ----------------
| bootstrap |<----| :ExtClassLoader |<----| :AppClassLoader |<----| :MyClassLoader |
-------------      -----------------       -----------------       ----------------
```

## The Python Standard Library
### Built-in Types
#### Sequence Types
There are seven sequence types: strings, Unicode strings, lists, tuples, bytearrays, buffers, and xrange objects.

##### String Methods
- str.join(iterable)
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Return a string which is the concatenation of the strings in *iterable*.
   ```
   >>> '_'.join(['aaa', 'bbb', 'ccc'])
   'aaa_bbb_ccc'
   ```

- `str.replace(old, new)`
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Return a copy of the string with all occurrences of substring *old* replaced by *new*.

- `str.split([sep])`
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Return a list of the words in the string, using *sep* as the delimiter string.
   ```
   >>> mary = 'Mary had a little lamb'
   >>> mary.split('a')
   ['M', 'ry h', 'd ', ' little l', 'mb']
   >>> mary.split()
   ['Mary', 'had', 'a', 'little', 'lamb']
   ```

- `str.startswith(prefix)`
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Return `True` if string starts with the *prefix*, otherwise return `False`.

##### String Formatting Operations
strings support the *format operator* `"%"`. Here is the general form:
```
format % values
```
If *format* requires a single argument, values may be a single non-tuple object. Otherwise, values must be a tuple with exactly the number of items specified by the format string.
```
>>> print "We have %d pallets of %s today." % (49, "kiwis")
We have 49 pallets of kiwis today.
```

## Generic Operating System Services
### `os`
This module provides a portable way of using operating system dependent functionality.
#### Process Parameters
These functions and data items provide information and operate on the current process and user.

- `os.getpid()`<br>
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Return the current process id.

   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Availability: Unix, Windows.

#### File Descriptor Operations
These functions operate on I/O streams referenced using file descriptors.

- `os.close(fd)`<br>
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Close file descriptor *fd*.

   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Availability: Unix, Windows.

- `os.fsync(fd)`<br>
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Force write of file with filedescriptor *fd* to disk. On Unix, this calls the native `fsync()` function.

   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Availability: Unix, and Windows starting in 2.2.3.

- `os.lseek(fd, pos, how)`<br>
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Set the current position of file descriptor *fd* to position *pos*, modified by *how*: `SEEK_SET` or `0` to set the position relative to the beginning of the file; `SEEK_CUR` or `1` to set it relative to the current position; `SEEK_END` or `2` to set it relative to the end of the file. Return the new cursor position in bytes, starting from the beginning.

   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Availability: Unix, Windows.

- `os.open(file, flags[, mode])`<br>
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Open the file *file* and set various flags according to *flags* and possibly its mode according to *mode*. Return the file descriptor for the newly opened file.

   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Availability: Unix, Windows.

- `os.write(fd, str)`<br>
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Write the string *str* to file descriptor *fd*. Return the number of bytes actually written.

   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Availability: Unix, Windows.

- `os.read(fd, n)`<br>
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Read at most *n* bytes from file descriptor *fd*. Return a string containing the bytes read. If the end of the file referred to by *fd* has been reached, an empty string is returned.
   
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Availability: Unix, Windows.

#### Files and Directories

- `os.access(path, mode)`<br>
  The function `access()` can be used to test the access rights a process has for a file. Return `True` if access is allowed, `False` if not.

#### Process Management
These functions may be used to create and manage processes.

- `os.system(command)`<br>
  It takes a single-string argument, which is the command line to be executed by a subprocess running a shell.
  ```Python
  import os

  # Simple command
  os.system('pwd')
  ```
  Since the command is passed directly to the shell for processing, it can include shell syntax such as globbing or environment variables.
  ```Python
  import os
  # Command with shell expansion
  os.system('echo $TMPDIR')
  ```
  The environment variable `$TMPDIR` in this string is expanded when the shell runs the command line.

  Unless the command is explicitly run in the background, the call to `system()` blocks until it is complete. Standard input, output, and error channels from the child process are tied to the appropriate streams owned by the caller by default.

### `argparse`
The `argparse` module makes it easy to write user-friendly command-line interfaces.
#### Example
##### Creating a parser
The first step in using the `argparse` is creating an `ArgumentParser` object:
```
>>> parser = argparse.ArgumentParser(description='Process some integers.')
```
##### Adding arguments
Filling an ArgumentParser with information about program arguments is done by making calls to the add_argument() method. Generally, these calls tell the ArgumentParser how to take the strings on the command line and turn them into objects. This information is stored and used when parse_args() is called. For example:
```
>>> parser.add_argument('integers', metavar='N', type=int, nargs='+',
...                     help='an integer for the accumulator')
>>> parser.add_argument('--sum', dest='accumulate', action='store_const',
...                     const=sum, default=max,
...                     help='sum the integers (default: find the max)')
```
##### Parsing arguments
`ArgumentParser` parses arguments through the `parse_args()` method. This will inspect the command line, convert each argument to the appropriate type and then invoke the appropriate action. In most cases, this means a simple Namespace object will be built up from attributes parsed out of the command line:
```
>>> parser.parse_args(['--sum', '7', '-1', '42'])
Namespace(accumulate=<built-in function sum>, integers=[7, -1, 42])
```
In a script, parse_args() will typically be called with no arguments, and the ArgumentParser will automatically determine the command-line arguments from `sys.argv`.

#### The `add_argument()` method
`ArgumentParser.add_argument(name or flags...[, action][, nargs][, default][, type][, help][, dest])`
Define how a single command-line argument should be parsed. Each parameter has its own more detailed description below, but in short they are:
- name or flags - Either a name or a list of option strings, e.g. `foo` or `-f, --foo`.
- action - The basic type of action to be taken when this argument is encountered at the command line.
- nargs - The number of command-line arguments that should be consumed.
- default - The value produced if the argument is absent from the command line.
- type - The type to which the command-line argument should be converted.
- help - A brief description of what the argument does.
- dest - The name of the attribute to be added to the object returned `by parse_args()`.

##### name or flags
ArgumentParser objects associate command-line arguments with actions. These actions can do just about anything with the command-line arguments associated with them, though most actions simply add an attribute to the object returned by `parse_args()`. The action keyword argument specifies how the command-line arguments should be handled. The supplied actions are:
- 'store' - This just stores the argument’s value. This is the default action. For example:
```
>>> parser = argparse.ArgumentParser()
>>> parser.add_argument('--foo')
>>> parser.parse_args('--foo 1'.split())
Namespace(foo='1')
```
-'store_true' and 'store_false' - These are special cases of 'store_const' using for storing the values True and False respectively. In addition, they create default values of False and True respectively. For example:
```
>>> parser = argparse.ArgumentParser()
>>> parser.add_argument('--foo', action='store_true')
>>> parser.add_argument('--bar', action='store_false')
>>> parser.add_argument('--baz', action='store_false')
>>> parser.parse_args('--foo --bar'.split())
Namespace(bar=False, baz=True, foo=True)
```

##### nargs
The supported values are:
- N (an integer).
```
>>> parser = argparse.ArgumentParser()
>>> parser.add_argument('--foo', nargs=2)
>>> parser.add_argument('bar', nargs=1)
>>> parser.parse_args('c --foo a b'.split())
Namespace(bar=['c'], foo=['a', 'b'])
```

- '*'. All command-line arguments present are gathered into a list.

##### dest
`ArgumentParser` generates the value of dest by taking the first long option string and stripping away the initial -- string.

dest allows a custom attribute name to be provided:
```
>>> parser = argparse.ArgumentParser()
>>> parser.add_argument('--foo', dest='bar')
>>> parser.parse_args('--foo XXX'.split())
Namespace(bar='XXX')
```

#### The parse_args() method
```
ArgumentParser.parse_args(args=None, namespace=None)
```
Convert argument strings to objects and assign them as attributes of the namespace. Return the populated namespace.

By default, the argument strings are taken from `sys.argv`, and a new empty Namespace object is created for the attributes.

##### The Namespace object
```
class argparse.Namespace
```
Simple class used by default by `parse_args()` to create an object holding attributes and return it.

### logging
#### Logger Objects
#### Logging Levels
#### Handler Objects
#### Formatter Objects

### logging.handlers
#### StreamHandler
#### FileHandler
#### SocketHandler

## Optional Operating System Services
### `threading`
This module defines the following functions and objects:

- `threading.Lock()`<br>
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;A factory function that returns a new primitive lock object.

- `class threading.Thread`<br>
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;A class that represents a thread of control.

#### Thread Objects
To implement a new thread using the threading module, you have to do the following:
- Define a new subclass of the `Thread` class
- Override the `__init__(self [,args])` method to add additional arguments
- Then, you need to override the `run(self [,args])` method to implement what the thread should do when it is started

Once you have created the new `Thread` subclass, you can create an instance of it and then start a new thread by invoking the `start()` method, which will, in turn, call the `run()` method.
```Python
import threading

class myThread(threading.Thread):
	def __init(self, threadID, name):
		threading.Thread.__init__(self)
		self.threadID = threadID
		self.name = name
	def run(self):
		print ("Starting " + self.name)
		print ("Exiting " + self.name)

# Create new threads
thread1 = myThread(1, "Thread-1")
thread2 = myThread(2, "Thread-2")

# Start new Threads
thread1.start()
thread2.start()
```
#### Lock Objects
- `Lock.acquire()`<br>
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Acquire a lock.

- `Lock.release()`<br>
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Release a lock.

## Interprocess Communication and Networking
### `socket`
This module provides access to the BSD *socket* interface.

The Python interface is a straightforward transliteration of the Unix system call and library interface for sockets to Python's object-oriented style: the `socket()` function returns a socket object whose methods implement the various socket system calls. Parameter types are somewhat higher-level than in the C interface: as with `read()` and `write()` operations on Python files, buffer allocation on receive operations is automatic, and buffer length is implicit on send operations.

#### Example
Here are four minimal example programs using the TCP/IP protocol: a server that echoes all data that it receives back (servicing only one client), and a client using it. Note that a server must perform the sequence `socket()`, `bind()`, `listen()`, `accept()` (possibly repeating the `accept()` to service more than one client), while a client only needs the sequence `socket()`, `connect()`.
```Python
# Echo server program
import socket

HOST = ''                 # Symbolic name meaning all available interfaces
PORT = 50007              # Arbitrary non-privileged port
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(1)
conn, addr = s.accept()
print 'Connected by', addr
while 1:
	data = conn.recv(1024)
	if not data: break
	conn.sendall(data)
conn.close()
```
```Python
# Echo client program
import socket

HOST = 'daring.cwi.nl'    # The remote host
PORT = 50007              # The same port as used by the server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
s.sendall('Hello, world')
data = s.recv(1024)
s.close()
print 'Received', repr(data)
```
Running an example several times with too small delay between executions, could lead to this error:
```
socket.error: [Errno 98] Address already in use
```
This is because the previous execution has left the socket in a `TIME_WAIT` state, and can't be immediately reused.

There is a socket flag to set, in order to prevent this, `socket.SO_REUSEADDR`:
```Python
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind((HOST, PORT))
```
the `SO_REUSEADDR` flag tells the kernel to reuse a local socket in `TIME_WAIT` state, without waiting for its natural timeout to expire.

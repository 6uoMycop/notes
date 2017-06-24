# Interprocess Communication

## Shared Memory
### Fast Local Communication
Shared memory is the fastest form of interprocess communication because all processes share the same piece of memory.Access to this shared memory is as fast as accessing a process's nonshared memory, and it does not require a system call or entry to the kernel. It also avoids copying data unnecessarily.

### Allocation
A process allocates a shared memory segment using `shmget`. Its first parameter is an integer key that specifies which segment to create. Unrelated processes can access the same shared segment by specifying the same key value. Unfortunately, other processes may have also chosen the same fixed key, which could lead to conflict.

Its second parameter specifies the number of bytes in the segment. Because segments are allocated using pages, the number of actually allocated bytes is rounded up to an integral multiple of the page size.

The third parameter is the bitwise or of flag values that specify options to `shmget`. The flag values include these:
- `IPC_CREAT` - This flag indicates that a new segment should be created.

If the call succeeds, `shmget` returns a segment identifier.

### Attachment and Detachment
To make the shared memory segment available, a process must use `shmat`. Pass it the shared memory segment identifier `SHMID` returned by `shmget`. The second argument is a pointer that specifies where in your process's address space you want to map the shared memory; if you specify NULL, Linux will choose an available address.

If the call succeeds, it returns the address of the attached shared segment.

When you're finished with a shared memory segment, the segment should be detached using `shmdt`. Pass it the address returned by `shmat`.

### Debugging
The `ipcs` command provides information on interprocess communication facilities, including shared segments. Use the `-m` flag to obtain information about shared memory.
```
% ipcs -m
------ Shared Memory Segments --------
key       shmid     owner     perms     bytes     nattch     status
0x00000000 1627649   user    640       25600     0
```
If this memory segment was erroneously left behind by a program, you can use the `ipcrm` command to remove it.
```
% ipcrm shm 1627649
```

## Pipes
A *pipe* is a communication device that permits unidirectional communication. Data written to the "write end" of the pipe is read back from the "read end." Pipes are serial devices; the data is always read from the pipe in the same order it was written. Typically, a pipe is used to communicate between two threads in a single process or between parent and child processes.

A pipe's data capacity is limited. If the writer process writes faster than the reader process consumes the data, and if the pipe cannot store more data, the writer process blocks until more capacity becomes available. If the reader tries to read but no data is available, it blocks until data becomes available.

### Creating Pipes
To create a pipe, invoke the `pipe` command. Supply an integer array of size 2. The call to `pipe` stores the reading file descriptor in array position 0 and the writing file descriptor in position 1. For example, consider this code:
```
int pipe_fds[2];
int read_fd;
int write_fd;

pipe(pipe_fds);
read_fd = pipe_fds[0];
write_fd = pipe_fds[1];
```
Data written to the file descriptor `read_fd` can be read back from `write_fd`.

### Communication Between Parent and Child Processes
A call to `pipe` creates file descriptors, which are valid only within that process and its children. When the process calls `fork`, file descriptors are copied to the new child process.

In the program, the parent writes a string to the pipe, and the child reads it out. The sample program converts these file descriptors into `FILE*` streams using `fdopen`.
```c
#include <stdlib.h>
#include <stido.h>
#include <unistd.h>

/* Write COUNT copies of MESSAGE to STREAM, pausing for a second
   between each. */

void writer (const char* message, int count, FILE* stream)
{
	for (; count > 0; --count)
	{
		/* Write the message to the stream, and send it off immediately. */
		fprintf(stream, "%s\n", message);
		fflush(stream);
		/* Snooze a while */
		sleep (1);
	}
}

/* Read random strings from the stream as long as possible. */

void reader (FILE* stream)
{
	char buffer[1024];
	/* Read until we hit the end of the stream. fgets reads until
	   either a newline or the end-of-file. */
	while (!feof(stream)
		   && !error(stream)
		   && fgest (buffer, sizeof(buffer), stream) != NULL)
		fputs(buffer, stdout);
}

int main()
{
	int fds[2];
	pid_t pid;

	/* Create a pipe. File descriptors for the two ends of the pipe are
	   placed in fds */
	pipe(fds);
	/* Fork a chile process. */
	pid = fork();
	if (pid == (pid_t)0) {
		FILE* stream;
		/* This is the child process. Close our copy of the write end of
		   the file descriptor. */
		close(fds[1]);
		/* Convert the read file descriptor to a FILE object, and read
		   from it. */
		stream = fdopen (fds[0], "r");
		reader (stream);
		close (fds[0]);
	}
	else {
		/* This is the parent process. */
		FILE* stream;
		/* Close our copy of the read end of the file descriptor. */
		close (fds[0]);
		/* Convert the write file descriptor to a FILE object, and write
		   to it. */
		stream = fdopen (fds[1], "w");
		writer ("Hello, world.", 5, stream);
		close (fds[1]);
	}

	return 0;
}
```
At the beginning of `main`, `fds` is declared to be an integer array with size 2. The `pipe` call creates a pipe and places the read and write file descriptors in that array. The program then forks a child process. After closing the read end of the pipe, the parent
process starts writing strings to the pipe.After closing the write end of the pipe, the
child reads strings from the pipe.

Note that after writing in the `writer` function, the parent flushes the pipe by calling `fflush`. Otherwise, the string may not be sent through the pipe immediately.

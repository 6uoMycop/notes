# Condition Variables
In particular, there are many cases where a thread wishes to check whether a **condition** is true before continuing its execution. For example, a parent thread might wish to check whether a child thread has completed before continuing; how should such a wait be implemented?
```c
void *child(void *arg) {
	printf("child\n");
	// XXX how to indicate we are done?
	return NULL;
}

int main(int argc, char *argv[])
{
	printf("parent: begin\n");
	pthread_t c;
	Pthread_create(&c, NULL, child, NULL); // create child
	// XXX how to wait for child?
	printf("parent: end\n");
	return 0;
}
```
We could try using a shared variable.
```c
volatile int done = 0;

void *child(void *arg) {
	printf("child\n");
	done = 1;
	return NULL;
}

int main(int argc, char *argv[])
{
	printf("parent: begin\n");
	pthread_t c;
	Pthread_create(&c, NULL, child, NULL); // create child
	while (done == 0)
		;  // spin
	printf("parent: end\n");
	return 0;
}
```
This solution will generally work, but it is hugely inefficient as the parent spins and wastes CPU time. What we would like here instead is some way to put the parent to sleep until the condition we are waiting for (e.g., the child is done executing) comes true.

## Definition and Routines
To wait for a condition to become true, a thread can make use of what is known as a **condition variable**. A **condition variable** is an explicit queue that threads can put themselves on when some state of execution (i.e., some **condition**) is not as desired (by **waiting** on the condition); some other thread, when it changes said state, can then wake one (or more) of those waiting threads and thus allow them to continue (by **signaling** on the condition).

To declare such a condition variable, one simply writes something like this: `pthread_cond_t c`; which declares `c` as a condition variable. A condition variable has two operations associated with it: `wait()` and `signal()`. The `wait()` call is executed when a thread wishes to put itself to sleep; the `signal()` call is executed when a thread has changed something in the program and thus wants to wake a sleeping thread waiting on this condition. Specifically, the POSIX calls look like this:
```
pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m);
pthread_cond_signal(pthread_cond_t *c);
```
We will often refer to these as `wait()` and `signal()` for simplicity. One thing you might notice about the `wait()` call is that it also takes a mutex as a parameter; it assumes that this mutex is locked when `wait()` is called. The responsibility of `wait()` is to release the lock and put the calling thread to sleep (atomically); when the thread wakes up (after some other thread has signaled it), it re-acquires the lock before returning to the caller.
```c
int done = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;

void thr_exit() {
	Pthread_mutex_lock(&m);
	done = 1;
	Pthread_cond_signal(&c);
	Pthread_mutex_unlock(&m);
}

void *child(void *arg) {
	printf("child\n");
	thr_exit();
	return NULL;
}

void thr_join() {
	Pthread_mutex_lock(&m);
	while (done == 0)
		Pthread_cond_wait(&c, &m);
	Pthread_mutex_unlock(&m);
}

int main(int argc, char *argv[])
{
	printf("parent: begin\n");
	pthread_t p;
	Pthread_create(&p, NULL, child, NULL);
	thr_join();
	printf("parent: end\n");
	return 0;
}
```
There are two cases to consider. In the first, the parent creates the child thread but continues running itself (assume we have only a single processor) and thus immediately calls into `thr_join()` to wait for the child thread to complete. In this case, it will acquire the lock, check if the child is done (it is not), and put it to sleep by calling `wait()` (hence releasing the lock). The child will eventually run, print the message "child", and call `thr_exit()` to wake the parent thread; this code just grabs the lock, sets the state variable `done`, and signals the parent thus waking it. Finally, the parent will run (returning from `wait()` with the lock held), unlock the lock, and print the message "parent: end".

In the second case, the child runs immediately upon creation, sets `done` to 1, calls signal to wake a sleeping thread (but there is none, so it just returns), and is done. The parent then runs, calls `thr_join()`, sees `done` is 1, and thus does not wait and returns.

To make sure you understand the importance of each piece of the `thr_exit()` and `thr_join()` code, let's try a few alternate implementations. First, you might be wondering if we need the state variable done. What if the code looked like the example below? Would this work?
```c
void thr_exit() {
	Pthread_mutex_lock(&m);
	Pthread_cond_signal(&c);
	Pthread_mutex_unlock(&m);
}

void thr_join() {
	Pthread_mutex_lock(&m);
	Pthread_cond_wait(&c, &m);
	Pthread_mutex_unlock(&m);
}
```
Unfortunately this approach is broken. Imagine the case where the child runs immediately and calls `thr_exit()` immediately; in this case, the child will signal, but there is no thread asleep on the condition. When the parent runs, it will simply call wait and be stuck; no thread will ever wake it. From this example, you should appreciate the importance of the state variable done; it records the value the threads are interested in knowing. The sleeping, waking, and locking all are built around it.

Here is another poor implementation. In this example, we imagine that one does not need to hold a lock in order to signal and wait. What problem could occur here?
```c
void thr_exit() {
	done = 1;
	Pthread_cond_signal(&c);
}

void thr_join() {
	if (done == 0)
		Pthread_cond_wait(&c);
}
```
The issue here is a subtle race condition. Specifically, if the parent calls `thr_join()` and then checks the value of `done`, it will see that it is 0 and thus try to go to sleep. But just before it calls wait to go to sleep, the parent is interrupted, and the child runs. The child changes the state variable `done` to 1 and signals, but no thread is waiting and thus no thread is woken. When the parent runs again, it sleeps forever, which is sad.

## The Producer/Consumer (Bounded Buffer) Problem
Imagine one or more producer threads and one or more consumer threads. Producers generate data items and place them in a buffer; consumers grab said items from the buffer and consume them in some way.

Because the bounded buffer is a shared resource, we must of course require synchronized access to it, lest a race condition arise. To begin to understand this problem better, let us examine some actual code.
```c
int buffer;
int count = 0; // initially, empty

void put(int value) {
	assert(count == 0);
	count = 1;
	buffer = value;
}

int get() {
	assert(count == 1);
	count = 0;
	return buffer;
}
```
Let's just use a single integer for simplicity.

Don't worry that this shared buffer has just a single entry; later, we'll generalize it to a queue that can hold multiple entries, which will be even more fun than it sounds. The `put()` routine assumes the buffer is empty
(and checks this with an assertion), and then simply puts a value into the shared buffer and marks it full by setting `count` to 1. The `get()` routine does the opposite, setting the buffer to empty (i.e., setting `count` to 0)
and returning the value.

This work is going to be done by two types of threads, one set of which we'll call the **producer** threads, and the other set which we'll call **consumer** threads.
```c
void *producer(void *arg) {
	int i;
	int loops = (int)arg;
	for (i = 0; i < loops; i++) {
		put(i);
	}
}

void *consumer(void *arg) {
	int i;
	while (1) {
		int tmp = get();
		printf("%d\n", tmp);
	}
}
```
The code for a producer that puts an integer into the shared buffer `loops` number of times, and a consumer that gets the data out of that shared buffer (forever), each time printing out the data item it pulled from the shared buffer.

Now we need to write some routines that know when it is OK to access the buffer to either put data into it or get data out of it. The conditions for this should be obvious: only put data into the buffer when `count` is zero (i.e., when the buffer is empty), and only get data from the buffer when count is one (i.e., when the buffer is full). If we write the synchronization code such that a producer puts data into a full buffer, or a consumer gets data from an empty one, we have done something wrong (and in this code, an assertion will fire).

**A Broken Solution**

Now imagine that we have just a single producer and a single consumer.

Let's examine the signaling logic between producers and consumers. When a producer wants to fill the buffer, it waits for it to be empty (p1–p3). The consumer has the exact same logic, but waits for a different condition: fullness (c1–c3).
```c
cond_t cond;
mutex_t mutex;

void *producer(void *arg) {
	int i;
	int loops = (int)arg;
	for (i = 0; i < loops; i++) {
		Pthread_mutex_lock(&mutex);            // p1
		if (count == 1)                        // p2
		    Pthread_cond_wait(&cond, &mutex);  // p3
		put(i);                                // p4
		Pthread_cond_signal(&cond);            // p5
		Pthread_mutex_unlock(&mutex);          // p6
	}
}

void *consumer(void *arg) {
	int i;
	for (i = 0; i < loops; i++) {
		Pthread_mutex_lock(&mutex);            // c1
		if (count == 0)                        // c2
		    Pthread_cond_wait(&cond, &mutex);  // c3
		int tmp = get();                       // c4
		Pthread_cond_signal(&cond);            // c5
		Pthread_mutex_unlock(&mutex);          // c6
		printf("%d\n", tmp);
	}
}
```
With just a single producer and a single consumer, the code in works. However, if we have more than one of these threads (e.g., two consumers), the solution has two critical problems. What are they?

Let's understand the first problem, which has to with the `if` statement before the wait. Assume there are two consumers (*T<sub>c1</sub>* and *T<sub>c2</sub>*) and one producer (*T<sub>p</sub>*). First, a consumer *T<sub>c1</sub>* runs; it acquires the lock (c1), checks if any buffers are ready for consumption (c2), and finding that none are, waits (c3) (which releases the lock).

Then the producer (*T<sub>p</sub>*) runs. It acquires the lock (p1), checks if all buffers are full (p2), and finding that not to be the case, goes ahead and fills the buffer (p4). The producer then signals that a buffer has been filled (p5). Critically, this moves the first consumer (*T<sub>c1</sub>*) from sleeping on a condition variable to the ready queue; *T<sub>c1</sub>* is now able to run (but not yet running). The producer then continues until realizing the buffer is full, at which point it sleeps (p6, p1-p3).

Here is where the problem occurs: another consumer (*T<sub>c2</sub>* ) sneaks in and consumes the one existing value in the buffer (c1, c2, c4, c5, c6, skipping the wait at c3 because the buffer is full). Now assume *T<sub>c1</sub>* runs; just before returning from the wait, it re-acquires the lock and then returns. It then calls `get()` (c4), but there are no buffers to consume! An assertion triggers, and the code has not functioned as desired. Clearly, we should have somehow prevented *T<sub>c1</sub>* from trying to consume because *T<sub>c2</sub>* snuck in and consumed the one value in the buffer that had been produced.

The problem arises for a simple reason: after the producer woke *T<sub>c1</sub>*, but *before* *T<sub>c1</sub>* ever ran, the state of the bounded buffer changed (thanks to *T<sub>c2</sub>*). Signaling a thread only wakes them up; it is thus a *hint* that the state of the world has changed (in this case, that a value has been placed in the
buffer), but there is no guarantee that when the woken thread runs, the state will still be as desired.

**Better, But Still Broken: While, Not If**

Fortunately, this fix is easy: change the `if` to a `while`. Think about why this works; now consumer *T<sub>c1</sub>* wakes up and (with the lock held) immediately re-checks the state of the shared variable (c2). If the buffer is empty at that point, the consumer simply goes back to sleep (c3). The corollary `if` is also changed to a `while` in the producer (p2).

Thanks to Mesa semantics, a simple rule to remember with condition variables is to **always use while loops**. Sometimes you don't have to re-check the condition, but it is always safe to do so.

However, this code still has a bug, the second of two problems mentioned above. It has something to do with the fact that there is only one condition variable.

The problem occurs when two consumers run first (*T<sub>c1</sub>* and *T<sub>c2</sub>*) and both go to sleep (c3).
Then, the producer runs, puts a value in the buffer, and wakes one of the consumers (say *T<sub>c1</sub>*). The producer then loops back (releasing and reacquiring the lock along the way) and tries to put more data in the buffer;
because the buffer is full, the producer instead waits on the condition (thus sleeping). Now, one consumer is ready to run (*T<sub>c1</sub>*), and two threads are sleeping on a condition (*T<sub>c2</sub>* and *T<sub>p</sub>*).

The consumer *T<sub>c1</sub>* then wakes by returning from `wait()` (c3), re-checks the condition (c2), and finding the buffer full, consumes the value (c4). This consumer then, critically, signals on the condition (c5), waking *only
one* thread that is sleeping. However, which thread should it wake?

**The Single Buffer Producer/Consumer Solution**

The solution here is once again a small one: use two condition variables, instead of one, in order to properly signal which type of thread should wake up when the state of the system changes.
```c
cond_t empty, fill;
mutex_t mutex;

void *producer(void *arg) {
	int i;
	int loops = (int)arg;
	for (i = 0; i < loops; i++) {
		Pthread_mutex_lock(&mutex);
		while (count == 1)
			Pthread_cond_wait(&empty, &mutex);
		put(i);
		Pthread_cond_signal(&fill);
		Pthread_mutex_unlock(&mutex);
	}
}

void *consumer(void *arg) {
	int i;
	for (i = 0; i < loops; i++) {
		Pthread_mutex_lock(&mutex);
		while (count == 0)
			Pthread_cond_wait(&fill, &mutex);
		int tmp = get();
		Pthread_cond_signal(&empty);
		Pthread_mutex_unlock(&mutex);
		printf("%d\n", tmp);
	}
}
```
In the code above, producer threads wait on the condition **empty**, and signals **fill**. Conversely, consumer threads wait on **fill** and signal **empty**. By doing so, the second problem above is avoided bu design: a consumer can never accidentally wake a consumer, and a producer can never accidentally wake a producer.

**The Correct Producer/Consumer Solution**

We now have a working producer/consumer solution, albeit not a fully general one. The last change we make is to enable more concurrency and efficiency; specifically, we add more buffer slots, so that multiple values can be produced before sleeping, and similarly multiple values can be consumed before sleeping.
```c
int buffer[MAX];
int fill_ptr = 0;
int use_ptr  = 0;
int count    = 0;

void put(int value) {
	buffer[fill_ptr] = value;
	fill_ptr = (fill_ptr + 1) % MAX;
	count++;
}

int get() {
	int tmp = buffer[use_ptr];
	use_ptr = (use_ptr + 1) % MAX;
	count--;
	return tmp;
}


cond_t empty, fill;
mutex_t mutex;

void *producer(void *arg) {
	int i;
	int loops = (int)arg;
	for (i = 0; i < loops; i++) {
		Pthread_mutex_lock(&mutex);             // p1
		while (count == MAX)                    // p2
		    Pthread_cond_wait(&empty, &mutex);  // p3
		put(i);                                 // p4
		Pthread_cond_signal(&fill);             // p5
		Pthread_mutex_unlock(&mutex);           // p6
	}
}

void *consumer(void *arg) {
	int i;
	for (i = 0; i < loops; i++) {
		Pthread_mutex_lock(&mutex);             // c1
		while (count == 0)                      // c2
		    Pthread_cond_wait(&fill, &mutex);   // c3
		int tmp = get();                        // c4
		Pthread_cond_signal(&empty);            // c5
		Pthread_mutex_unlock(&mutex);           // c6
		printf("%d\n", tmp);
	}
}
```
A producer only sleeps if all buffers are currently filled (p2); similarly, a consumer only sleeps if all buffers are currently empty (c2).

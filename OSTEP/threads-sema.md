# Semaphores
## Semaphores: A Definition
A semaphore is an object with an integer value that we can manipulate with two routines; these routines are `sem_wait()` and `sem_post()`. Because the initial value of the semaphore determines its behavior, before calling any other routines to interact with the semaphore, we must first initialize it to some value.
```C
#include <semaphore.h>
sem_t s;
sem_init(&s, 0, 1);
```
We declare a semaphore and initialize it to the value 1 by passing 1 in as the third argument. The second argument to `sem_init()` will be set to 0 in all of the examples we'll see; this indicates that the semaphore is shared between threads in the same process. See the man page for details on other usages of semaphores (namely, how they can be used to synchronize access across *different* processes), which require a different value for that second argument.

After a semaphore is initialized, we can call one of two functions to interact with it, `sem_wait()` or `sem_post()`.
```
int sem_wait(sem_t *s) {
	decrement the value of semaphore s by one
	wait if value of semaphore s is negative
}

int sem_post(sem_t *s) {
	increment the value of semaphore s by one
	if there are one or more threads waiting, wake one
}
```
First, we can see that `sem_wait()` will either return right away (because the value of semaphore was one or higher when we called `sem_wait()`), or it will cause the caller to suspend execution waiting for a subsequent post. Of course, multiple calling threads may call into `sem_wait()`, and thus all be queued waiting to be woken.

Second, we can see that `sem_post()` does not wait for some particular condition to hold like `sem_wait()` does. Rather, it simply increments the value of the semaphore and then, if there is a thread waiting to be woken, wakes one of them up.

Don't worry (yet) about the seeming race conditions within the semaphore; assume that the actions they make are performed atomically.

## Binary Semaphores (Locks)
We are now ready to use a semaphore. Our first use will be one with which we are already familiar: using a semaphore as a lock.
```C
sem_t m;
sem_init(&m, 0, 1);

sem_wait(&m);
// critical section here
sem_post(&m);
```
To make this clear, let's imagine a scenario with two threads. The first thread (Thread 0) calls `sem_wait()`; it will first decrement the value of the semaphore, changing it to 0. Then, it will wait only if the value is *not* greater than or equal to 0. Because the value is 0, `sem_wait()` will simply return and the calling thread will continue; Thread 0 is now free to enter the critical section. If no other thread tries to acquire the lock while Thread 0 is inside the critical section, when it calls `sem_post()`, it will simply restore the value of the semaphore to 1 (and not wake a waiting thread, because there are none).

A more interesting case arises when Thread 0 "holds the lock" (i.e., it has called `sem_wait()` but not yet called `sem_post()`), and another thread (Thread 1) tries to enter the critical section by calling `sem_wait()`. In this case, Thread 1 will decrement the value of the semaphore to -1, and thus wait (putting itself to sleep and relinquishing the processor). When Thread 0 runs again, it will eventually call `sem_post()`, incrementing the value of the semaphore back to zero, and then wake the waiting thread (Thread 1), which will then be able to acquire the lock for itself. When Thread 1 finishes, it will again increment the value of the semaphore, restoring it to 1 again.

Note in particular that Thread 1 goes into the sleeping state when it tries to acquire the already-held lock; only when Thread 0 runs again can Thread 1 be awoken and potentially run again.

Thus we are able to use semaphores as locks. Because locks only have two states (held and not held), we sometimes call a semaphore used as a lock a **binary semaphore**.

## How To Implement Semaphores
Finally, let's use our low-level synchronization primitives, locks and condition variables, to build our own version of semaphores called ... **Zemaphores**.
```C
typedef struct __Zem_t
{
	int value;
	pthread_cond_t cond;
	pthread_mutex_t lock;
} Zem_t;

// only one thread can call this
void Zem_init(Zem_t* s, int value) {
// void Zem_init(Zem_t *s, unsigned int value);
	s->value = value;
	Cond_init(&s->cond);
	Mutex_init(&s->lock);
}

void Zem_wait(Zem_t *s) {
	Mutex_lock(&s->lock);
	while (s->value <= 0 )
		Cond_wait(&s->cond, &s->lock);
	s->value--;
	Mutex_unlock(&s->lock);
}

void Zem_post(Zem_t *s) {
	Mutex_lock(&s->lock);
	s->value++;
	Cond_signal(&s->cond);
	Mutex_unlock(&s->lock);
}
```
As you can see from the figure, we can just use one lock and one condition variable, plus a state variable to track the value of the semaphore.

# POSIX multi-threading

C++ does not contain any built-in support for multithreaded applications. Instead, it relies entirely upon the operating system to provide this feature.

This tutorial assumes that you are working on Linux OS and we are going to write multi-threaded C++ program using POSIX. POSIX Threads, or Pthreads provides API which are available on many Unix-like POSIX systems such as GNU/Linux, Mac OS X.

{% callout "POSIX" %}
The Portable Operating System Interface (POSIX) is a family of standards specified by the IEEE Computer Society for maintaining compatibility between operating systems. POSIX defines the application programming interface (API), along with command line shells and utility interfaces, for software compatibility with variants of Unix and other operating systems.
{% endcallout %}

## Creating threads 

The following routine is used to create a POSIX thread

```cpp
#include <pthread.h>
pthread_create (thread, attr, start_routine, arg) 
```



Here, `pthread_create` creates a new thread and makes it executable. This routine can be called any number of times from anywhere within your code. The arguments of the function are the following

* `thread`: a unique identifier for the new thread returned by the subroutine
* `attr`: an attribute object that may be used to set thread attributes. You can specify NULL for the default values. 	
* `start_routine`: the C++ routine that the thread will execute once it is created
* `arg`: A single argument that may be passed to start_routine. It must be passed by reference as a pointer cast of type void. NULL may be used if no argument is to be passed

The maximum number of threads that may be created by a process is implementation dependent. Once created, threads are peers, and may create other threads. **There is no implied hierarchy or dependency between threads**.

## Terminating threads

To terminate a POSIX thread, do

```cpp
#include <pthread.h>
pthread_exit (status) 
```

Here `pthread_exit` is used to explicitly exit a thread. Typically, the `pthread_exit()` routine is called after a thread has completed its work and is no longer required to exist.

If `main()` finishes before the threads it has created, and exits with `pthread_exit()`, the other threads will continue to execute. Otherwise, they will be automatically terminated when `main()` finishes.



## Example 

This simple example code creates 5 threads with the `pthread_create()` routine. Each thread prints a "Hello World!" message, and then terminates with a call to `pthread_exit()`. 

```cpp
#include <iostream>
#include <cstdlib>
#include <pthread.h>

using namespace std;

#define NUM_THREADS 5

void *PrintHello(void *threadid) {
   long tid;
   tid = (long)threadid;
   cout << "Hello World! Thread ID, " << tid << endl;
   pthread_exit(NULL);
}

int main () {
   pthread_t threads[NUM_THREADS];
   int rc;
   int i;
   
   for( i = 0; i < NUM_THREADS; i++ ) {
      cout << "main() : creating thread, " << i << endl;
      rc = pthread_create(&threads[i], NULL, PrintHello, (void *)i);
      
      if (rc) {
         cout << "Error:unable to create thread," << rc << endl;
         exit(-1);
      }
   }
   pthread_exit(NULL);
}
```

Compile the following program using -lpthread library as follows 

```cpp
g++ test.cpp -lpthread
```

Now, execute your program which gives the following output

```cpp
[rbertens@tatooiine tutorial]$ ./a.out 
main() : creating thread, 0
main() : creating thread, 1
Hello World! Thread ID, 0
main() : creating thread, 2
Hello World! Thread ID, 1
main() : creating thread, 3
main() : creating thread, 4
Hello World! Thread ID, 2
Hello World! Thread ID, 3
Hello World! Thread ID, 4
```

As you can see, the threads executed in no particular order. 

## Passing Arguments to Threads

This example shows how to pass multiple arguments via a structure. You can pass any data type in a thread callback because it points to void as explained in the following example

```cpp
#include <iostream>
#include <cstdlib>
#include <pthread.h>

using namespace std;

#define NUM_THREADS 5

struct thread_data {
   int  thread_id;
   char *message;
};

void *PrintHello(void *threadarg) {
   struct thread_data *my_data;
   my_data = (struct thread_data *) threadarg;

   cout << "Thread ID : " << my_data->thread_id ;
   cout << " Message : " << my_data->message << endl;

   pthread_exit(NULL);
}

int main () {
   pthread_t threads[NUM_THREADS];
   struct thread_data td[NUM_THREADS];
   int rc;
   int i;

   for( i = 0; i < NUM_THREADS; i++ ) {
      cout <<"main() : creating thread, " << i << endl;
      td[i].thread_id = i;
      td[i].message = "This is message";
      rc = pthread_create(&threads[i], NULL, PrintHello, (void *)&td[i]);
      
      if (rc) {
         cout << "Error:unable to create thread," << rc << endl;
         exit(-1);
      }
   }
   pthread_exit(NULL);
}
```

When the above code is compiled and executed, it produces the following result −

```cpp
[rbertens@tatooiine tutorial]$ ./a.out 
main() : creating thread, 0
main() : creating thread, 1
Thread ID : 0 Message : This is message
main() : creating thread, 2
Thread ID : 1 Message : This is message
main() : creating thread, 3
Thread ID : 2 Message : This is message
main() : creating thread, 4
Thread ID : 3 Message : This is message
Thread ID : 4 Message : This is message
```

As we learned before, void pointers are very powerful! 

## Joining and detaching threads

As we have seen before, threads execute independent from one another. For a program flow however, you might want to make sure that a given thread (or a pool of threads) have executed, before your program continues. Here, **joining** and **detaching** comes in handy. 

A thread can run in two modes: 

- **Joinable mode (default)**. A joinable thread will not release any resource even after the end of thread function, until some other thread calls `pthread_join()` with its ID. 
- Detached mode. A Detached thread automatically releases it allocated resources on exit. No other thread needs to join it. 


The detached attribute merely determines the behavior of the system when the thread terminates; it does not prevent the thread from being terminated if the process terminates using exit (or equivalently, if the main thread returns). The pthread_detach() function marks the thread identified by thread as detached. When a detached thread terminates, its resources are automatically released back to the system without the need for another thread to join with the terminated thread.


There are following two routines which we can use to join or detach threads

```cpp
pthread_join (threadid, status) 
pthread_detach (threadid) 
```

In practice, an example of using joinable threads is e.g.

```cpp
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
 
#include <unistd.h>
 
void * threadFunc(void * arg)
{
	std::cout << "Thread Function :: Start" << std::endl;
	// Sleep for 2 seconds
	sleep(2);
	std::cout << "Thread Function :: End" << std::endl;
	// Return value from thread
	return new int(6);
}
 
int main()
{
	// Thread id
	pthread_t threadId;
 
	// Create a thread that will funtion threadFunc()
	int err = pthread_create(&threadId, NULL, &threadFunc, NULL);
	// Check if thread is created sucessfuly
	if (err)
	{
		std::cout << "Thread creation failed : " << strerror(err);
		return err;
	}
	else
		std::cout << "Thread Created with ID : " << threadId << std::endl;
	// Do some stuff
 
	void * ptr = NULL;
	std::cout << "Waiting for thread to exit" << std::endl;
	// Wait for thread to exit
	err = pthread_join(threadId, &ptr);
	if (err)
	{
		std::cout << "Failed to join Thread : " << strerror(err) << std::endl;
		return err;
	}
 
	if (ptr)
		std::cout << " value returned by thread : " << *(int *) ptr
				<< std::endl;
 
	delete (int *) ptr;
	return 0;
}
```

conversely, with detached threads, one can write

```cpp
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
 
#include <unistd.h>
 
void * threadFunc(void * arg)
{
	std::cout << "Thread Function :: Start" << std::endl;
	// Sleep for 2 seconds
	sleep(2);
	std::cout << "Thread Function :: End" << std::endl;
	// Return value from thread
	return new int(6);
}
 
int main()
{
	// Thread id
	pthread_t threadId;
 
	// Create a thread that will funtion threadFunc()
	int err = pthread_create(&threadId, NULL, &threadFunc, NULL);
	// Check if thread is created sucessfuly
	if (err)
	{
		std::cout << "Thread creation failed : " << strerror(err);
		return err;
	}
	else
		std::cout << "Thread Created with ID : " << threadId << std::endl;
	// Do some stuff
 
	void * ptr = NULL;
	std::cout << "Waiting for thread to exit" << std::endl;
	// Wait for thread to exit
	err = pthread_join(threadId, &ptr);
	if (err)
	{
		std::cout << "Failed to join Thread : " << strerror(err) << std::endl;
		return err;
	}
 
i
	if (ptr)
		std::cout << " value returned by thread : " << *(int *) ptr
				<< std::endl;
 
	delete (int *) ptr;
	return 0;
}
```

{% callout "OpenMP and C++11 std::thread" %}

POSIX mutli threading is powerful, but not exactly easy. Two other approaches are mentioned here for completeness, they are easier to implement. 

### Thread support in std in C++11 

Since C++11, the std library offers thread support:

```cpp
void thread_main() {
    std::cout << "Hello, World (thread)" << std::endl;
}

int main() {
    std::thread t1(thread_main);
    std::cout << "Hello, World (main)" << std::endl;
    t1.join();
    return 0;
}
```

### OpenMP

A relatively easy way to achieve multi threading is using OpenMP, which uses compiler pragma's to do the dirty work for you:
```cpp
#include <stdio.h>
#include <omp.h>

int main(void)
{
    #pragma omp parallel
    printf("Hello, world.\n");
    return 0;
}

```
when compiling the above code snippet, prepend `-fopenmp` as compiler flag! The pragma directives will be expanded in proper C++ code before translation into machine code. Hence, executing the above code on e.g. a 4 core machine would result in

```cpp
Hello, world.
Hello, world.
Hello, world.
Hello, world.
```

(although, since the threads all share the same standard output, it's more likely that you end up in a race condition and see something like `Hello, wHello, woorld.rld.Hello, wHello, woorld.rld.`....)



{% endcallout %}


# Exercises

First of all, try to go through the examples that were shown in the lecture: copy the code snippets, and try to compile and execute them. You can also
- give the threads some more work, e.g. make a computationally intensive loop
- use your system monitor to keep track of CPU/core usage
- can you demonstrate that multi threading actually speeds up execution? 

Secondly, we have talked a lot about race conditions and deadlocks
- can you write your own race condition? Hint: problems often occur when one thread does a "check-then-act" (e.g. "check" if the value is X, then "act" to do something that depends on the value being X) and another thread does something to the value in between the "check" and the "act". E.g:

```cpp
if (x == 5) // The "Check"
{
   y = x * 2; // The "Act"

   // If another thread changed x in between "if (x == 5)" and "y = x * 2" above,
   // y will not be equal to 10.
}
```

The point being, y could be 10, or it could be anything, depending on whether another thread changed x in between the check and act. You have no real way of knowing.

Alternatively, you can create a race condition (or deadlock) by giving the threads access to shared memory (e.g. global variables or structs). Write a program that breaks itself! That will serve as an excellent motivation for the next session: handing over the headaches of multi threading to ROOT. 

{% challenge "Hint ..." %}
Below a program that creates a race condition
{% solution "Solution" %}
```cpp
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <iostream>
#include <unistd.h>

using namespace std;

int num1 = 0;
int num2 = 0;
bool raceDetected;
unsigned int seed = 0x000fff;
const int NUM_THREADS = 2;
pthread_mutex_t mutex;

void * thread_routine (void *thread) {

	int *id_ptr, thread_num;

   	id_ptr = (int *) thread;
   	thread_num = *id_ptr;

	int counter = 0;
	int tmp1;
	int tmp2;
	int r;

	do {
		tmp1 = num1;
  		tmp2 = num2;
		r = rand_r ( &seed ) % 11;
  		num1 = tmp1 + r;
  		num2 = tmp2 - r;
  		counter++;
		} while ( num1 + num2 == 0 && !raceDetected );
	raceDetected = true;

	pthread_mutex_lock(&mutex);
	cout << "counter: " << counter << " - thread number: " << thread_num  << endl;
	pthread_mutex_unlock(&mutex);
}


int main() {

	pthread_t tid[NUM_THREADS];
	int* threadIdNum[NUM_THREADS];

	for (int i=0; i<NUM_THREADS; i++ ) {
		threadIdNum[i] = new int;
		*threadIdNum[i] = i;
    		if ( pthread_create ( &(tid[i]), NULL, thread_routine, (void *) threadIdNum[i] )) {
      		cout << "thread create failed!\n";
      		exit(1);
    		}
	}

	do {
	0 + 0;
	}  while( !raceDetected );

	sleep(1000000);
	return 0;
}
```

If you run it, printouts to screen are given when a race condition is detected. To fix it, we can introduce mutexes for the critical section of the thread:

```cpp
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <iostream>
#include <iomanip>
#include <time.h>
#include <unistd.h>

using namespace std;

int num1 = 0;
int num2 = 0;
bool raceDetected = false;
unsigned int seed = 0x000fff;
const int NUM_THREADS = 2;
const int MAX_RUN_COUNT = 50000000;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
bool finished[NUM_THREADS];

timespec timeDifference(timespec start, timespec end)   
{
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

void * thread_routine (void *thread) {

	int *id_ptr, thread_num;

	id_ptr = (int *) thread;
	thread_num = *id_ptr;

	long counter = 0;
	int tmp1;
	int tmp2;
	int r;
	while(counter < MAX_RUN_COUNT && !raceDetected) {
		//Entry
		pthread_mutex_lock (&mutex);

		//Critical Section
		tmp1 = num1;
		tmp2 = num2;
		r = rand_r ( &seed ) % 5;
		num1 = tmp1 + r;
		num2 = tmp2 - r;
		counter++;

		if(num1+num2 != 0 || raceDetected)  // detect race
		{
			raceDetected = true;
			cout << num1 << '\t' << num2 << '\t' << num1+num2 << endl;
			cout << counter << " : " << (raceDetected ? "true" : "false") << " : " << thread_num  << endl;
			raceDetected = true;
		}

		//Exit
		pthread_mutex_unlock(&mutex);
		//Remainder
	}

	finished[thread_num] = true;
}


int main() {

	pthread_t tid[NUM_THREADS];	//threads
	int* threadIdNum[NUM_THREADS];	//thread id number that will be passed to thread when pthread_create is called
	int returnCode;
	pthread_attr_t attr;
	void *status;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	timespec beginTimer, stopTimer;

	clock_gettime(CLOCK_MONOTONIC, &beginTimer);

	for (int i=0; i<NUM_THREADS; i++ ) {
		threadIdNum[i] = new int;	//create new int for thread number
		*threadIdNum[i] = i;		//intialize thread number
		returnCode = pthread_create ( &(tid[i]), &attr, thread_routine, (void *) threadIdNum[i] );	//create thread and output message if it fails
      		if (returnCode) {
         		cerr << "ERROR; return code from pthread_join() is " << returnCode << endl;
	         	exit(-1);
		} else {
			cout << "MAIN: completed thread creation of thread " << i << " successfully" << endl;
		}
	}

	pthread_attr_destroy(&attr);

	cout << "\nMAIN: threads running...\n\n";

	for(int i=0; i<NUM_THREADS; i++) {
      		returnCode = pthread_join( tid[i], &status);

      		if (returnCode) {
         		cerr <<"ERROR; return code from pthread_join() is " << returnCode << endl;
	         	exit(-1);
		} else {
			cout << "MAIN: completed join with thread " << i << " having a status of " << (long)status << endl;
		}
	}

	clock_gettime(CLOCK_MONOTONIC, &stopTimer);

	cout << "MAIN: all threads completed\n\n";

	cout <<  num1 << " + " << num2 << " = " << num1 + num2 << endl;
	cout << "time: " << timeDifference(beginTimer, stopTimer).tv_sec << "." << timeDifference(beginTimer, stopTimer).tv_nsec << " seconds" << endl;

	return 0;
}
```

{% endchallenge %}


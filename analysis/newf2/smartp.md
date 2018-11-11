# Smart pointers

In the previous section, we have talked about pointers and memory management. As you probably know, dynamic memory management can bring quite some headaches, such as

- memory leaks, when allocated memory is not deallocated
- ownership issues: who is in charge of deallocating memory? 

Luckily, C++11 comes with **smart pointers**, aimed at helping you out in exactly those situations. 

{% callout "Pointers" %}
A pointer is an object whose value “points to” another value stored
somewhere else in memory
-   it contains a **memory address**
-   obtaining the stored at the pointed location
-   very flexible and powerful tool
{% endcallout %}

## Pitfalls of pointers
Let's look at some use cases of pointers:
 

{% challenge "Declaring a pointer - what's wrong? " %}
```cpp
    int* a; // declares a pointer that can point to an integer value
```
{% solution "Solution" %}
The pointer points to a random address in memory! To avoid this, always initialize to NULL:
```cpp    
    int* b = nullptr; // OK, pointer is initialised to a null memory address
```
{% endchallenge %}

We can use pointers to allocate memory on the heap
```cpp
    int* c = new int; // allocate memory for an integer value in the heap
    //and assign its memory address to this pointer
```
{% challenge " What does this do?" %}
```cpp
    int** d = &a;
```
{% solution "Solution" %}
This points to a pointer ...
```cpp    
    int** d = &a; // this pointer points to a pointer to an integer value
```
Because, why not?
{% endchallenge %}

And then there are of course pointers to objects 

```cpp
    MyObject* e = new MyObject(); // allocate memory for MyObject
    // and assign its memory address to the pointer e
    
    /* Using a pointer */
    int f = *c; // dereferencing a pointer and assigning the pointed
    // value to another integer variable
    
    e->DoSomething(); // dereferencing a pointer and calling
    // the method DoSomething() of the instance of MyObject
    // pointed by e
```



## Memory leaks

memory leak is a type of resource leak that occurs when a computer program incorrectly manages memory allocations in such a way that memory which is no longer needed is not released. A memory leak may also happen when an object is stored in memory but cannot be accessed by the running code. A memory leak has symptoms similar to a number of other problems and generally can only be diagnosed by a programmer with access to the program's source code. 

{% challenge " What is the problem with this code?" %}
Below is a snippet of analysis code. What is wrong with it ? 
```cpp
    void MyAnalysisTask::UserExec()
    {
      TLorentzVector* v = nullptr;
      for (int i = 0; i < InputEvent()->GetNumberOfTracks(); i++) {
        AliVTrack* track = InputEvent()->GetTrack(i);
        if (!track) continue;
        v = new TLorentzVector(track->Px(), 
          track->Py(), track->Pz(), track->M());
    
        // my analysis here
        std::cout << v->Pt() << std::endl;
      }
  
      delete v;
    }
```

{% solution "Solution" %}

Look at the line
```cpp
v = new TLorentzVector(track->Px(), 
```

For each iteration the loop, a new instance of `TLorentzVector` is created. Only the last instance is deleted via the `delete` call at the end of the code. The memory of the other instances is not deleted, and cannot be deleted anymore (the pointers to it are overwritten in the loop). 
{% endchallenge %}

## Array or single value?
Besides being at a risk of memory leaks, pointers can lead to truly ambiguous situations when using *arrays*, since a pointer can point to a single value or to an array, there is  no way to infer it from its declaration. There is a different syntax to destroy (= deallocate, free) the pointed object for arrays and single objects as we have seen in the previous section. Look at the following snippet of code, that illustrates this ambiguity: 

```cpp
    AliVTrack* FilterTracks();

    void UserExec()
    {
      TLorentzVector *vect = new TLorentzVector(0,0,0,0);
      double *trackPts = new double[100];
      AliVTrack *returnValue = FilterTracks();

      // here use the pointers

      delete vect;
      delete[] trackPts;
      delete returnValue; // or should I use delete[] ??
    }
```

## Ownership issues: double deletes

-   Each memory allocation should match a corresponding deallocation

-   Difficult to keep track of all memory allocations/deallocations in a
    large project

-   **Ownership** of the pointed memory is ambiguous: multiple deletes
    of the same object may occur

```cpp
    AliVTrack* FilterTracks();
    void AnalyzeTracks(AliVTrack* tracks);

    void MyAnalysisTask::UserExec()
    {
      AliVTrack* tracks = FilterTracks();

      AnalyzeTracks(tracks);

      delete[] tracks; // should I actually delete it?? 
      //or was it already deleted by AnalyzeTracks?
    }
```


# Smart Pointers


-   Clear (*shared* or *exclusive*) **ownership** of the pointed object

-   memory is deallocated when the last pointer goes out of scope

-   Available since C++11

## Exclusive-Ownership Pointers: `unique_ptr`

-   Automatic garbage collection with (i.e. it uses the same
    resources as a raw pointer)

-   `unique_ptr` **owns** the object it points

-   Memory automatically released when `unique_ptr` goes out of scope or
    when its `reset(T* ptr)` method is called

-   Only one `unique_ptr` can point to the same memory address

{% callout "Unique pointers in the wild" %}

Example 1

```cpp
    void MyFunction() {
      std::unique_ptr<TLorentzVector> vector(new TLorentzVector(0,0,0,0));
      std::unique_ptr<TLorentzVector> vector2(new TLorentzVector(0,0,0,0));
  
      // use vector and vector2
  
      // dereferencing unique_ptr works exactly as a raw pointer
      std::cout << vector->Pt() << std::endl;
  
      // the line below does not compile!
      // vector = vector2;
      // cannot assign the same address to two unique_ptr instances
  
      vector.swap(vector2); // however I can swap the memory addresses
  
      // this also releases the memory previously pointed by vector2
      vector2.reset(new TLorentzVector(0,0,0,0)); 
  
      // objects pointed by vector and vector2 are deleted here
    }
```

Example 2

```cpp
    void MyAnalysisTask::UserExec()
    {
      for (int i = 0; i < InputEvent()->GetNumberOfTracks(); i++) {
        AliVTrack* track = InputEvent()->GetTrack(i);
        if (!track) continue;
        std::unique_ptr<TLorentzVector> v(new TLorentzVector(track->Px(), 
          track->Py(), track->Pz(), track->M()));
    
        // my analysis here
        std::cout << v->Pt() << std::endl;
        // no need to delete
        // v is automatically deallocated after each for loop
      }
    }
```

No memory leak here! :)

{% endcallout %}





## Shared-Ownership Pointers: `shared_ptr`

-   Automatic garbage collection with some CPU and memory overhead

-   The pointed object is *collectively owned* by one or more
    `shared_ptr` instances

-   Memory automatically released the last `shared_ptr` goes out of
    scope or when it is re-assigned

![SharedPtr](Sharedptr.png)

{% callout "Shared pointers in the wild" %}

Example 1

```cpp
    void MyFunction() {
      std::shared_ptr<TLorentzVector> vector(new TLorentzVector(0,0,0,0));
      std::shared_ptr<TLorentzVector> vector2(new TLorentzVector(0,0,0,0));
  
      // dereferencing shared_ptr works exactly as a raw pointer
      std::cout << vector->Pt() << std::endl;
  
      // assignment is allowed between shared_ptr instances
      vector = vector2; 
      // the object previously pointed by vector is deleted!
      // vector and vector2 now share the ownership of the same object
  
      // object pointed by both vector and vector2 is deleted here
    }
```

Example 2

```cpp
    class MyClass {
      public:
        MyClass();
      private:
        void MyFunction();
        std::shared_ptr<TLorentzVector> fVector;
    };

    void MyClass::MyFunction() {
      std::shared_ptr<TLorentzVector> vector(new TLorentzVector(0,0,0,0));
  
      // assignment is allowed between shared_ptr instances
      fVector = vector;
      // the object previously pointed by fVector (if any) is deleted
      // vector and fVector now share the ownership of the same object

      // here vector goes out-of-scope
      // however fVector is a class member so the object is not deleted!
      // it will be deleted automatically when this instance of the class
      // is deleted (and therefore fVector goes out-of-scope) :)
    }
```
{% endcallout %}

## Some word of caution on `shared_ptr'


{% challenge "What is the problem in this case?" %}
You have crated smart code, using smart pointers. It looks like this
```cpp
    void MyClass::MyFunction() {
      auto ptr = new TLorentzVector(0,0,0,0);
  
      std::shared_ptr<TLorentzVector> v1 (ptr);
      std::shared_ptr<TLorentzVector> v2 (ptr);
  
    }
```
still, something is wrong with it, do you know what ?

{% solution "Solution" %}

A possible way of solving this:

```cpp
    void MyFunction() {
      auto ptr = new TLorentzVector(0,0,0,0);
  
      std::shared_ptr<TLorentzVector> v1 (ptr);
      std::shared_ptr<TLorentzVector> v2 (ptr);
  
      // a double delete occurs here!
    }
```

-   `v1` does not know about `v2` and viceversa!

-   Two control blocks have been created for the same pointed objects

Some word of caution on `shared_ptr`

```cpp
    void MyFunction() {
      std::shared_ptr<TLorentzVector> v1 (new TLorentzVector(0,0,0,0));
      std::shared_ptr<TLorentzVector> v2 (v1);
  
      // this is fine!
    }
```

The lesson to learn: use raw pointers only when absolutely needed (if at all)

{% endchallenge %}

# Exercise

Below is a very long page of exercises related to using smart pointers, varying in difficulty. Click on 'solution' to see the answer to the questions. You can put the questions 'in action' by writing small C++ programs, but they are also answerable by just inspecting the code snippets.  

{% challenge "Question" %}
The following contains a mistake.  Fix it, and explain why it was wrong.
```c++
std::shared_ptr<int> p1(new int(42));
std::shared_ptr<int> p2(p1.get());
```
{% solution "Solution" %}
`p1` and `p2` will both attempt to delete the same pointer, without knowing the other exists.  A `shared_ptr` counts the number of pointers referencing the same pointer and deallocates that pointer once no one is referencing it.  With code like the previous, we get the following:
```c++
std::shared_ptr<int> p1(new int(42));//p1 references the new pointer, reference count = 1
std::shared_ptr<int> p2(p1.get());//p2 thinks it's the only guy looking at the pointer, reference count = 1
//(automatically called) p2.~shared_ptr<int>() -- deletes the pointer in p2
//(automatically called) p1.~shared_ptr<int>() -- Oops, p1's pointer was already deleted!
```

The correct way to ensure the reference counts are correct is to copy the shared_ptr, not just the underlying pointer:
```c++
std::shared_ptr<int> p1(new int(42));//p1 references the new pointer, reference count = 1
std::shared_ptr<int> p2(p1);//p2 references p1's pointer & p1's reference count, reference count = 2
//(automatically called) p2.~shared_ptr<int>() -- decrements the reference count ( = 1 )
//(automatically called) p1.~shared_ptr<int>() -- decrements the reference count to zero and deletes the pointer.
```
{% endchallenge %}


{% challenge "Question" %}
The following contains a mistake.  Fix it, and explain why it was wrong.
```c++
std::unique_ptr<int> p3(new int[42]);
```
{% solution "Solution" %} 
`std::unique_ptr<T>` will call `delete` on its pointer -- that means the pointer should be allocated using `new`.  If we want to use `new[N]`, we should store it in `std::unique_ptr<T[]>`, which will correctly call `delete[]`.  Furthermore, it offers `operator[]` and disallows `operator*` & `operator->`, which makes it less awkward to index elements after the first.

{% endchallenge %}


{% challenge "Question" %}
When would you use a `unique_ptr` versus a `shared_ptr`?
{% solution "Solution" %}
Use `shared_ptr` when you know that ownership of the pointer must be shared in more than one place.  Note that _ownership_ == _responsability to deallocate the pointer_.
Use `unique_ptr` when you don't know that ownership of the pointer must be shared.
{% endchallenge %}


{% challenge "Question" %}
Suppose you have a class which returns a pointer which the user _may_ wish to store in a `shared_ptr`.  What type should you return?
{% solution "Solution" %}
Return a `unique_ptr` to that type.  `unique_ptr`s are convertible to `shared_ptr`s, so returning a `unique_ptr` prevents memory leaks without restricting how that pointer is used later.
Returning a raw pointer instead would certainly work, but goes against, e.g., Scott Meyers's advice to "make interfaces easy to use correctly and hard to use incorrectly."  (See Item 18 of Effective C++.)
{% endchallenge %}



{% challenge "Question" %}
The following code opens a handle to a file using the C-style `FILE*` interface
and closes the handle before returning.
```c++

  FILE* pFin = NULL;
  try
  {
    std::string filename("c:\\Windows\\win.ini");
    pFin = std::fopen(filename.c_str(), "r");
    if (!pFin)
      return;

    char data[10];
    if (std::fread(data, 1, 10, pFin) <= 0)
    {
      std::fclose(pFin);
      return;
    }

    std::fclose(pFin);
  }
  catch (...)
  {
    if (pFin)
      std::fclose(pFin);
    throw;  //rethrow after closing file
  }
```
We could change it to use
iostreams, but for this exercise, change the code to just use a smart
pointer (`std::shared_ptr`) to `FILE` with a custom deleter, so that the
`fclose()` call doesn't have to appear in so many places, and eliminate the
try-catch block.
{% solution "Solution" %}:
```c++
    std::string filename("c:\\Windows\\win.ini");
    std::shared_ptr<FILE> pFin(std::fopen(filename.c_str(), "r"), &std::fclose);
    if (!pFin)
      return;

    char data[10];
    if (std::fread(data, 1, 10, pFin.get()) <= 0)
    {
      return;
    }
```
Notice that not only do the `fclose()` calls go away, but also the try/catch block goes away, since there's no longer a need to call `fclose()` and then re-throw.
{% endchallenge %}


{% challenge "Question" %}
```c++
auto p_unique = get_unique_ptr(my_lib.open_handle(),
[&](int* p_handle) { my_lib.close_handle(p_handle); });

//Now suppose we need to change to a shared_ptr:
std::shared_ptr<int> p_shared(p_unique.release());
```
What's wrong with the conversion to `shared_ptr` and why does it cause a
crash?
{% solution "Solution" %}
The custom deleter is not transferred from the `unique_ptr` to the `shared_ptr` when you use `release()`.  Therefore, when the `shared_ptr` goes out of scope, it calls delete on the raw pointer instead of the custom deleter.
{% endchallenge %}


{% challenge "Question" %}
The fix for the above looks like this:
```c++
std::shared_ptr<int> p_shared(std::move(p_unique));
```
Rewrite the code above. Why do you have to use `std::move()`?
{% solution "Solution" %}:
`unique_ptr` cannot be copied, as that would not provide unique ownership
of the pointer.  `unique_ptr` can be moved from.  This allows `unique_ptr` to
be returned from functions, used in std containers, and more.  `std::move`
explicitly moves the pointer out of `p_unique` into `p_shared`.
{% endchallenge %}

{% challenge "Question" %}
When we copied a `shared_ptr` to another `shared_ptr`, we didn't have to use
`std::move()`. Why not?
{% solution "Solution" %}
Because `shared_ptrs` are copy assignable whereas `unique_ptr`s are not.  With
a `unique_ptr`, we have to help transfer ownership through the move because
ownership can not be copied.

It seems that explicitly calling `std::move` is usually only necessary when
writing move constructors and move assignment operators.  (There are other
cases, like when writing algorithms, but moves are usually safest
(and fastest!) when implicitly generated.)
{% endchallenge %}

{% challenge "Question" %}
```c++
//We have a unique_ptr to const and we want to switch to a unique_ptr to
// non-const:
std::unique_ptr<const int> p_const_unique(new int(5));
//std::unique_ptr<int> p_my_non_const_int(std::move(p_const_unique));
```
If you uncomment the line above, it won't compile.  Why not?
{% solution "Solution" %}
Just as with raw pointers, converting a pointer to `const` to a pointer to
non-`const` will fail.

From a higher-level point of view, if this conversion were allowed, 
then the following could happen.
```c++
std::unique_ptr<const int> p_const_unique(new int(5));
std::unique_ptr<int> p_my_non_const_int(std::move(p_const_unique));
// We are changing the value of something originally declared const!
*p_my_non_const_int = 4;
```
{% endchallenge %}

{% challenge "Question" %}
Consider the following code:
```c++
int foo() { throw std::runtime_error("Normally conditional");  return 5; }

void bar(std::shared_ptr<int> int, int val)
{}

void example2()
{
  bar(std::shared_ptr<int>(new int(42)), foo());
}
```
Why could calling example2 result in a memory leak?
{% solution "Solution" %}
C++ arguments may be evaluated in unspecified order.  In the previous, the `shared_ptr` constructor and `foo()` must both be evaluated before `bar` is called, but there is no requirement that the `shared_ptr` constructor be called before `foo()` is evaluated.  Thus the order of operations could be:
`new int(42)`
`foo()`
and the `shared_ptr` constructor might never be called, as `foo()` threw and exception.
{% endchallenge %}

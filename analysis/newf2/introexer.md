# welcome to the exercise part

{% challenge " Question " %}
Considering only performance, when should you pass small objects by value, reference and pointer?
```c++
struct SmallStruct { int i };

void foo(SmallStruct s);
void foo(SmallStruct& s);
void foo(SmallStruct* s);
```
{% solution " Solution " %}
There was no significant performance difference between passing small objects (or built-ins) by value, reference, or pointer.
{% endchallenge %}



{% challenge " Question " %}
Considering only performance, when should you pass large objects by value, reference and pointer?
```c++
struct LargeStruct { std::vector<int> i };

void foo(LargeStruct s);
void foo(LargeStruct& s);
void foo(LargeStruct* s);
{% solution " Solution " %}

```
Passing by value is the slowest since Foo's copy constructor is invoked for each call. Passing by reference or by pointer essentially copy a machine word — there's no significant difference.
{% endchallenge %}

{% challenge " Question " %}
Are there reasons to prefer one argument-passing mechanism to another?
{% solution " Solution " %}
The difference between writing `->` versus `.` is trivial. More important to writing clear code is the semantic difference between them.  Consider the following code:
    ```c++
void foo(MyLargObj* p)
{
    p->bar();
}
```
Perhaps when you first wrote `foo`, it was called in one place, and you knew that `p` would always be valid.  Maybe you even wrote a comment for `foo` inidcating the precondition that `p` should never be `NULL`.  Inevitably, someone (maybe you) will pass `NULL`.  You might then feel the need to check `p` after all.
    ```c++
void foo(MyLargObj* p)
{
    if (!p)
        return;
    p->bar();
}
```
This can lead to other problems -- what if a post condition of `foo()` is that the postcondition of `MyLargObj::bar` hold?  Code executed later might still fail.  You might decide that throwing an exception is more appropriate.  You could get concerned about throwing an exception when checking a precondition and check on the internet for when it's appropriate.  You could be reading differing viewpoints on it for years, and if you don't immediately agree that you should `assert`, you'll probably do the wrong thing ;).

No matter what you choose, that choice was necessitated by the fact that pointers may assume invalid values. So, if passing in a value is truly optional, passing by pointer may be appropriate. If a precondition of the function requires no invalid values, consider pass-by-reference or pass-by-value — this more explicitly declares the domain of a given function.  Foo becomes:
    ```c++
void foo(MyLargObj& p)
{
    p.bar();
}
```
Which declares via the input type that invalid values for `p` are not allowed.  Passing by value may also be reasonable, but it depends on if changes to an object should be visible after the function is complete.

A final consideration is that pointers carry a notion of ownership. Pointers often point to non-stack allocated objects (e.g., allocated using new), and should be deallocated when no longer needed. Lots of comments can be written indicating ownership — "Ownership must be passed to this function, it will call `delete`," etc. Using references or values can reduce such error-prone commenting burden since they convey no notion of ownership. (Another way to address this, smart pointers, will be covered later. The discussion on the semantic differences between passing by reference and by value still applies in that case, however.)
{% endchallenge %}


{% challenge " Question " %}
What happens when passing a derived object as a base object?
```c++
class Base
{
    public:
        virtual ~Base() {}
        virtual void foo() const { std::cout << "Base" << std::endl; }
};
class Derived : public Base
{
    public:
        virtual void foo() const { std::cout << "Derived" << std::endl; }
};

void bar1(Base b) { b.foo(); }
void bar2(Base& rB) { rB.foo(); }
void bar3(Base* pB) { pB->foo(); }
...
Derived d;
bar1(d);
bar2(d);
bar3(&d);
```
{% solution " Solution " %}
All calls do not produce identical output — when a derived object is passed by value, only the 'base' copy constructor is called. Things that are not copied include the ability to call derived object virtual functions. This is known as the slicing problem. 

{% endchallenge %}



{% challenge " Question " %}
What are the performance implications of returning large objects by value, reference and pointer?
    ```c++
Foo return_by_value()
{ return Foo(); }
Foo& return_by_reference()
{
    //KLUDGE: We can't return a reference to an object whose lifetime ends
    // after this function exits. Therefore we declare a local static.
    // There are even kludgier methods for dealing with this, however.
    static Foo s_f;
    s_f = Foo();
    return s_f;
}
Foo* return_by_pointer()
{ return new Foo; }
```
{% solution " Solution " %}
There are significant differences between return by value, reference and pointer; those differences may be surprising. Return by value was the fastest in my runs (.83 s), return by reference the slowest (1.37 s), and return by pointer in between (.90 s). All three require the construction and destruction of a Foo object. It may be useful to examine the differences:

Return by value: You might think this would be the slowest, arguing that it also requires a copy construction. That would have been true several years ago if optimizations were disabled on this code. In this case, the compiler noticed that the object constructed inside the function was going to be destructed after its values were copied. It was able to give the array of the to-be-destroyed object directly to the Foo object in the calling code [^2].

Return by reference: As written in the exercise, this is the slowest — perhaps surprisingly so. Its slowness is due to synchronization required when initializing local static variables. (The synchronization, aside from making high-latency calls, quadruples the number of instructions executed.)

Return by pointer: This is sometimes faster than return by value, sometimes slower — but usually around the same speed. This doesn't require the synchronization of return by reference, but does require a small heap allocation.

The details mentioned here aren't particularly important. The point of this exercise is that returning a large object by value may be the correct decision; concerns about the speed of returning by value are often unfounded and avoiding it is a case of premature pessimization. 

{% endchallenge %}



{% challenge " Question " %}
What are the correctness concerns of using a local static to return by reference?
    ```c++
Foo& return_by_reference()
{
    //KLUDGE: We can't return a reference to an object whose lifetime ends
    // after this function exits. Therefore we declare a local static.
    // There are even kludgier methods for dealing with this, however.
    static Foo s_f;
    s_f = Foo();
    return s_f;
}

Foo& f1 = return_by_reference();
Foo& f2 = return_by_reference();
std::cout << "Are f1 and f2 equal? "
<< std::boolalpha << (f1 == f2) << std::endl; //Outputs true
```
{% solution " Solution " %}
If the first entry of f1 and f2 are different when printed, why do they compare equal? Since f1 refers to the same object as f2, f1 and f2 will always compare equal. The point? Local static (non-const) objects shouldn't be used as a way to avoid return by value. It is both wrong and slow.
{% endchallenge %}

{% challenge " Question " %}
Is there a difference in speed when stateful objects are involved?
```c++
struct Foo
{
    Foo() : v(10000) {};
    std::vector<int> return_by_value() const { return v; }
    const std::vector<int>& return_by_reference() const { return v; }
    const std::vector<int>* return_by_pointer() const { return &v; }
    private:
    std::vector<int> v; 
};
```
{% solution " Solution " %}
The timings are very different when returning an existing object. Return by value was 5000x slower than the other methods. In this case, return by value MUST call the copy constructor, while return by reference or pointer needed only return the address.
{% endchallenge %}

{% challenge " Question " %}
How should we decide between return by reference or pointer?
{% solution " Solution " %}
If the object to be returned may not exist, return by pointer is natural, as NULL may serve as a sentinel value. Otherwise, return by reference is usually preferred, as that clearly indicates the object is valid. (Again, we'll ignore smart pointers and skip a discussion of ownership.)
{% endchallenge %}

{% challenge " Question " %}
What are the consequences of returning a local polymorphic object by value, reference and pointer?

{% solution " Solution " %}
Returning an object created in a function by value produces incorrect results as described in Q4. Returning by reference may produce correct results, but is more fragile. If a static object is referenced, the issues described in Q6 are present. Others returned \*(new Derived()), which will produce correct results and avoid the slowness of the static object; however, the complexity of ensuring that referenced object is destructed correctly is still reason enough to avoid it. Return by pointer should be preferred in this case.
{% endchallenge %}


{% challenge " Question " %}
When should we pass by value, reference and pointer?
{% solution " Solution " %}
Pass small objects	by value: Yes, if not polymorphic and changes inside the function should not be present outside the function
Pass small objects by reference: Yes, if no need for a sentinel value
Pass small objects by pointer: Yes, if a sentinel value is needed

Pass large objects by value: Only if the function actually needs a separate copy of the object
Pass large objects by reference: Yes, if no need for a sentinel value
Pass large objects by pointer: Yes, if a sentinel value is needed

Pass polymorphic objects by value: No
Pass polymorphic objects by reference: Yes, if no need for a sentinel value
Pass polymorphic objects by pointer: Yes, if a sentinel value is needed
{% endchallenge %}


{% challenge " Question " %}
When should we return by value, reference and pointer?
{% solution " Solution " %}
Return small objects by value: Yes, if not polymorphic
Return small objects by reference: Only if the returned object's will exist after the function is exited 
Return small objects by pointer: Yes, if a sentinel value is needed or the object cannot be on the stack 

Return large objects by value: Yes, unless the object's lifetime exceeds the function call, or measurements show that this impacts performance
Return large objects by reference Yes, if no need for a sentinel value and the object's lifetime exceeds the function call.
Return large objects by pointer: Yes, if a sentinel value is needed and the object's lifetime exceeds the function call

Return polymorphic objects by value: No
Return polymorphic objects by reference: Yes, if no need for a sentinel value and object lifetime exceeds function scope
Return polymorphic objects by pointer: Yes, if a sentinel value is needed or return by reference is otherwise inappropriate
{% endchallenge %}

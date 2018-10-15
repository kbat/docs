---
title: "Basic code debugging"
layout: tweet

createtoc: true
parnumbers: true
---

Some general programming tips
-----------------------------

In order to reduce as much as possible the amount of time spent debugging and
making your code faster, here are some general tips you might find useful.


### Use existing algorithms for general problems

A large portion of coding problems applied to data collections can be reduced
to **sorting** or **searching**: even if a simple custom made array sorting
code looks innocuous and easy to read, when iterating over a collection of
thousands or millions of elements its drawbacks become pretty obvious.

As a general rule, **do not write your own sorting algorithms**: standard ROOT
data collections, like [TLists](http://root.cern.ch/root/html/TList.html) and
[TObjArrays](http://root.cern.ch/root/html/TObjArray.html), already have an
efficient `Sort()` method.

Moreover, if you are manually filling a TList for sorting it at a later time,
consider using something like the
[TSortedList](http://root.cern.ch/root/html/TSortedList.html), which is
compatible with a TList, *i.e.* you can do:

```c++
TList *mySortedList = new TSortedList();
```

but objects are inserted in the correct order every time you `Add()` them,
avoiding additional time waste to order it at a later time.

The same considerations apply for **searching algorithms**: ROOT already have
search functions optimized for its data structures, and if you search in a
sorted list (like the TSortedList), search is faster.

Searching by strings inside large collections **must be avoided at all times**:
string comparison is a very expensive operation, and it is not easy to estimate
how long it takes since it depends on the length of the compared strings.
Massive string comparison, even if this is unobvious, leads to:

* **large data transfers** between CPU, caches and memory, hitting the bus
  limits
* **cache trashing**, *i.e.* the constant invalidation of the caches that are
  supposed to speed up code execution

A bad example that uses massively string comparisons has been found inside ROOT
thanks to the IgProf profiler, and the code has been amended, as we will see
later on.

To improve lookups in large collections, please consider
[THashList](http://root.cern.ch/root/html/THashList.html) instead of TList: a
"hash" is a fixed-size byte sequence (usually an integer) associated to each
element and computed based on the content of the element, constituting a sort
of "fingerprint".

When performing a lookup on a hash list (*i.e.* find what element is identical
to this one, or find what element has this name), the hash ("fingerprint") of
the input element is calculated, and it is compared with the hash of every
other element in the list.

If two hashes match, it *might* mean that the two elements are identical: in
this case, and this case only, a deep check is performed and the response is
returned.

The big advantage is that from a shallow (hash) comparison we can tell for sure
if two elements are different, which saves us the time required to perform a
deep comparison for every element.

> Use optimized versions of the algorithms even if they are difficult to
> understand, and **trust them**!

For a list of optimized general purpose algorithms in C and C++, have a look at
the [Numerical Recipes](http://www.nr.com/), which is considered the "sacred
text" of this field.


### Object ownership

Very often, the ownership of pointers is not clear, *i.e.* who created a
certain object, and who is responsible of garbage collecting it?

If you are the person defining a new object, be sure to do something like:

```c++
AliMyObject *mo = new AliMyObject();
// code code code
delete mo;
```

You have probably heard it lots of times already, but it is worth repeating it:
write a `delete` every time you write a `new`, and think in advance of how to
dispose of an object *you* created.

ROOT collections (*i.e.* the TLists) can be made "owners" of objects:

```c++
TList *l = new TList();
l->SetOwner(kTRUE);
// add add add
delete l;
```

This means that when the code reaches the `delete` line, `delete` is
automatically invoked on each of the contained objects, so that you do not have
to do it manually.


### New and delete, and hidden memory operations

Please note that **new and delete operations are very slow**: never ever create
and destroy an object per loop (for instance inside the `UserExec()` of the
analysis tasks)!

> **Save memory and CPU, recycle objects!** And if the object classes
> are defined by you, write them in a way that you can **redefine**
> their content without the need to delete and recreate them!

A **bad example** with objects constructed per loop:

```c++
for (x=0; x<100; x++) {
  for (y=0; y<100; y++) {
    TLorentzVector vec1( x, y, 0, 0);
    TLorentzVector vec2(-x, y, 0, 0);
    TLorentzVector res = vec1 + vec2;
    vec1.Boost( -res.GetBoostVector() );
    // ...
  }
}
```

Even if `new` and `delete` are not used there (objects are on the stack), the
`TLorentzVectors` have their constructor and destructor invoked **per loop!**

The amended code:

```c++
TLorentzVector vec1;
TLorentzVector vec2;
TLorentzVector res;

for (x=0; x<100; x++) {
  for (y=0; y<100; y++) {
    vec1.SetXYZT( x, y, 0, 0);
    vec2.SetXYZT(-x, y, 0, 0);
    res = vec1 + vec2;
    vec1.Boost( -res.GetBoostVector() );
    // ...
  }
}
```

[As you can see](http://root.cern.ch/root/html/TLorentzVector.html), the
`TLorentzVector` has various `Set*()` methods that make it not necessary to
destroy and recreate the object to set new coordinates.

Also, **do not be too zealous when deleting**: in particular, do not delete
objects that do not belong to you.

> Common example: do not delete the object if you get its pointer from a
> `TDirectory` (like a `TFile`)!

Choose the ROOT container carefully: even if a `TList` and a `TObjArray` are
both collections capable of expanding, **lists are sparse** in memory, as each
element points to the next (and sometimes to the previous); instead, **arrays
are contiguous collections of elements** in memory.

Growing an array requires periodic **hidden** `realloc()` low level operations
(luckily they do not happen for every element we add), and this might be rather
impairing when applied to a very large collection: use a `TList` in such a
case.


### ROOT I/O and objects

A note about ROOT objects (*i.e.* objects inheriting from `TObject`, *i.e.*
most likely **your own** objects): ROOT as you know needs a default constructor
for I/O that takes no arguments. You should create at least **two
constructors** in your code for clarity:

* The default one, needed by ROOT, that should be called only by ROOT when
  storing or restoring object from a file, and should never be used by the
  programmer directly: **do not write any memory allocation instruction there**,
  leave it as empty as possible!
* The "explicit" one, taking at least one argument, which should do the actual
  stuff and should be used by the programmer when creating a new object.

Histograms or trees obtained from a file with statements like:

```c++
TH1F *histo = (TH1F *)myFile->Get("myHist");
```

are related to the corresponding `TFile`, and they are **destroyed** when the
file is closed. So, **do not do that**:

```c++
TFile *myFile = TFile::Open("myfile.root");
TH1F *histo = (TH1F *)myFile->Get("myHist");
delete myFile;
histo->Draw();
```

This is because **TFile owns the pointer**. You can detach the histogram from
the file:

```c++
TFile *myFile = TFile::Open("myfile.root");
gROOT->cd();
TH1F *histo = (TH1F *)myFile->Get("myHist");
delete myFile;
histo->Draw();
```

but in this case **you own the histogram** and **you must dispose of it**:

```c++
// when done...
delete histo;
```

Note that you could also use `histo->SetDirectory(0)` for being able to close
the file, but it is better to attach it to the "ROOT root directory" in order
to use it for instance in `TTree::Draw`.

Please note that there are in particular **two objects** that are automatically
associated to a `TFile`, mostly for performance reasons:

* Histograms (*i.e.* all objects inheriting from `TH1`)
* Trees (*i.e.* all objects inheriting from `TTree`, including the `TNtuple`)

For those categories of objects, the corresponding `TFile` owns the object. You
must always **disassociate** the object from the originating `TFile` if you
want to keep the object while closing the file.

> This does not apply for objects that are neither histograms nor trees: in
> that case (*e.g* a `TList` or a `TGraph`) **you** are responsible of
> disposing of the object.


### Compiler warnings

**Do not overlook compiler warnings!** Warnings should **never** exist in a
clean code.

Whenever you see a warning, you have to:

* **fix** the code, if it is actually broken: sometimes warnings get it right
  that you are doing something wrong
* **silence** the warning by rewriting the snippet in a way that it does not
  trigger the warning.

Different compilers give different warnings: clang is usually more precise and
tends to give you advice on either how to silence the warning, or what you
probably meant when writing the code. Of course a compiler is a "stupid" piece
of software and you should not always follow literally its advice, but do not
overlook it!

Two good examples clarifying why warnings should not be ignored:

* Starting from GCC 5 many former warnings are turned into errors by default. We
  have evaluated the opportunity to pass GCC 5 some flags to maintain the legacy
  behavior (*i.e.* revert some errors into warnings) but we have found out that
  the compiler was right: those warnings were actually faulty code that needed
  to be fixed. If the authors checked the warnings in the first place they would
  have for sure recognized the logic errors in the code.
* Warnings clog compilation output and make more difficult to detect actual
  errors when they occur.

Note that by default aliBuild conceals the compilation output. You need to pass
the `--debug` option to actually see the warnings popping up:

```bash
aliBuild build AliPhysics --debug
```


### When to use pointers

As a general rule, it is not always a good idea to use pointers to objects as
data members of your class. If objects are big you are forced to use it, but
for many small objects you'd better use the object directly: you will save
yourself (and the computer) a new/delete operation for each one of them. This
has an impact if you have many small objects.

Remember to always initialize pointers used as data members: compilers should
raise warnings if you do not.

Many member pointers in custom classes are **transient**, *i.e.* they should
exist only in memory and they should not be written to a file.  So please mark
every transient pointer with the appropriate special ROOT comment `//!` (no
spaces between the slash and exclamation mark):

```c++
class MyClass : public TObject {
  private:
    AnotherClass *transient_ptr;  //!
  // ...
}
```

> This comment is not just a decorator: you are telling ROOT **not to save** the
> pointer to file, considerably reducing the output size!


### Strings

A very very very bad habit found very frequently in analysis code is
referencing objects via their name inside the `UserExec()` (or any other loop):

```c++
TH1I *h = list->FindObject("myHistoName");
```

The `FindObject()` function is very (very!) expensive, furthermore it always
returns the same pointer for a certain object: it surely makes no sense to run
it inside the `UserExec()`! Do this call **once** during the initialization and
use the **cached** result in the event loop!

Also, avoid passing "raw" strings as arguments to functions. Don't do:

```c++
void MyClass::myfunc(TString a) {
  // ...
}
```

Do instead:

```c++
void MyClass::myfunc(const TString &a) {
  // ...
}
```

Note that in both cases you invoke it with:

```c++
myfunc("this is a string");
```

which means that you can update your code without changing the way you call
such functions.

Moreover, it is a **bad idea** to use strings as triggering options on a method,
like:

```c++
void MyClass::dosomething(const TString &what) {
  if (what == "this") {
    // ...
  }
  else if (what == "that") {
    // ...
  }
  else if (what == "something else") {
    // ...
  }
}
```

As we have seen, **string comparison is evil**. Use **enums** instead:

```c++
enum dowhat_t { THIS, THAT, SOMETHING_ELSE };

void MyClass::dosomething(const dowhat_t what) {
  if (what == THIS) {
    // ...
  }
  else if (what == THAT) {
    // ...
  }
  else if (what == SOMETHING_ELSE) {
    // ...
  }
}
```

The code is as readable, but much more efficient.

# Implicit typing

C++ is a strongly typed programming language.
Therefore, each variable needs to have a well defined data type.
Since C++11 the compiler can infer this type from the context of the program, which saves lots of typing or typedef declarations.

```cpp
// Not that useful:
auto inumber  = 200;                // implicit integer
auto fnumber  = 1.6;                // implicit double
auto text     = "my first poem";    // implicit const char *

// More useful:
auto histogram = new TH1F("h", "h", 100, 0, 100); // more useful

// Very useful:
std::vector<int> v{33,78,994};
for (std::vector<int>::iterator it = v.begin(); it != v.end(); ++it) {}
for (auto it = v.begin(); it != v.end(); ++it) {}
```

The handyness of the `auto` keyword becomes even more obvious in meta programming, where deducing of the type can become very tedious:

```cpp
template<typename T, typename S>
void foo(T lhs, S rhs) {
  auto product = lhs * rhs;
  //...
}
```
Here, the type of `product` can be different for each instanciation of the templated function.

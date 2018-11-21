# Move Semantics

Before C++11 returning a large object was inefficient since it required making a copy of the whole object.

```cpp
class Foo {
  public:
    Foo(int i) : val(i) {}
    std::vector<int> val;
};
Foo getBigFoo() { Foo foo(100000); return foo; }
int main() {
  Foo foo = getBigFoo();
}
```

This sparked the idea to reuse the already allocated content of the existing object created in the function, instead of wasting resources by cloning the whole object (which will not be used afterwards, anyway).
Therefore, the concept of rvalue references (specified with `&&`) was introduced.
An rvalue is a temporary object that does not have a name.
In C++11 returned objects are automatically rvalues and two new special class member functions are defined:
the move constructor and the move assignment operator.

```cpp
Foo(Foo&& other) ...
const Foo& operator= (Foo&& other) ...
```
Move constructors are available for all STL containers.

The efficient transfer of resources from one object to another can explicitly be triggered with the `std::move` function.

```cpp
std::string foo = "foo-string";
std::string bar = "bar-string";
std::vector<std::string> myvector;
myvector.push_back (foo);
myvector.push_back (std::move(bar));
std::cout << "foo is: " << foo << '\n'; // copies
std::cout << "bar is: " << bar << '\n'; // moves
std::cout << "myvector contains:";
for (const auto& x:myvector) std::cout << ' ' << x;
std::cout << '\n';
```

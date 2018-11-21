# Strongly typed enums

Classical C++ enumerators do not have their own scope.
Therefore each name can only be used once:

```cpp
enum Colors {Green, Yellow, Orange};
enum Fruits {Apple, Banana, Orange};  // error: redefinition of enumerator 'Orange'
```
With strongly typed enumerators, this is no longer an issue, since each enumerator has a separate namespace:

```cpp
enum class Colors {Green, Yellow, Orange};
enum class Fruits {Apple, Banana, Orange}; // no problem
```

Another disadvantage of classical enumerators is that they allow comparison of the elements in two different enums. This can cause strange and unwanted behaviour:

```cpp
if(Banana == Yellow) cout << "This is apples and oranges, but it works!!" << endl;
// warning: comparison of two values with different enumeration
```
For strongly typed enumerators this would result in a compiler error:

```cpp
if(Fruits::Banana == Colors::Yellow) cout << "I will not compile this nonsense!" << endl;
//error: invalid operands to binary expression
```

Since C++11 it is also possible to specify the underlying type of the (classical and strongly typed) enums:

```cpp
enum Letters : char { A, B, C };
enum class Numbers : int { A, B, C };
```

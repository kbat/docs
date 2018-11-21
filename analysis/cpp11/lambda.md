# lambda expressions
The new C++11 feature of lambda expressions provides the possibility to create unnamed function objects capable of capturing variables in scope.
```cpp
[ captures ] ( params ) -> ret { body }
```
The capture list `[]` allows to specify what is visible in the function body:

`[]`           : no variables visible

`[var1, var2]` : listed variables

`[&]`          : all variables by reference

`[=]`          : all variables by value

Specifying a return type is optional.
The type of the lambda function itself is unique and unnamed:

```cpp
  auto f = [](int a, int b) {return a * b;};

  decltype(f) g = f; // decltype determines the type of f
  cout << g(2, 3) << endl;
```

In-place function definitions can be useful as parameters for other functions:

```cpp
std::vector<int> v{-8,3,5};
std::sort(v.begin(), v.end(), [](int x, int y) { return abs(x) < abs(y); });
```

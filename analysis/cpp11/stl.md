# Selection of new features in the standard library

## std::array
Provides a container of fixed size and type.
Can be accessed via the `[]` operator similar to C arrays.

```cpp
std::array<std::string, 4> strings = {"How", "are", "you", "doing"};
strings[0] = "What";
for(auto word : strings) cout << word << " ";
cout << "?" << endl;
```
## std::tuple
A tuple provides the possibility to obtain a fixed-size collection of heterogeneous values.
This can for instance be useful when a function shall return more than one value:

```cpp
std::tuple<int, std::string, float> foo_tuple()
{
  return std::make_tuple(1, "abc", -1.3);
}

auto retValue = foo_tuple();
cout << std::get<0>(retValue) << endl;
cout << std::get<1>(retValue) << endl;
cout << std::get<2>(retValue) << endl;


```

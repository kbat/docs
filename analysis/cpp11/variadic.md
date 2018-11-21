# Variadic templates

C++ 11 allows for a variable number of template function parameters:

```cpp
template<typename... Args>
void FillHistograms(std::vector<Histo> &histos, Args... args) {
  for (auto &h : histos) h.fill(args...);
}
 ```
Here `typename... Args` is called a template parameter pack,
and `Args... args` is called a function parameter pack
(Args is, of course, a completely arbitrary name and could be anything else).
The pack is then expanded using `args...`.

With this one can for for example write a template function for a recursive adder:

```cpp
 template<typename T>
 T adder(T v) {
   return v;
 }

 template<typename T, typename... Args>
 T adder(T first, Args... args) {
   return first + adder(args...);
 }
```

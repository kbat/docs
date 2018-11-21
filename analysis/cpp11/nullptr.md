# nullptr

The `nullptr` is a new literal of type `std::nullptr_t` introduced in C++11.
It provides a specific expression for null pointers.
Before C++11, it was common to use `0`, `0x0` or the preprocessor macro `NULL` to indicate a pointer to nothing.
This is not type save as the following example illustrates:

```cpp
void printMe(int i) { cout << "I am an integer: " << i << endl; }
void printMe(int *i) { cout << "I am a pointer to an integer: " << i << endl; }

int main() {
  printMe(0);
  printMe(NULL);      // error: call to 'printMe' is ambiguous
  printMe(nullptr);   // works fine
}
```
To be backward compatible, you can:
`#define nullptr NULL`
(ROOT already does this for you).

#include <iostream>
#include <vector>
#include <array>
#include <tuple>
#include <string>
using std::cout;
using std::endl;

template<typename T>
T adder(T v) {
  return v;
}

template<typename T, typename... Args>
T adder(T first, Args... args) {
  std::cout << __PRETTY_FUNCTION__ << "\n";
  return first + adder(args...);
}

int main() {

  adder(1, 2, 3, 8, 7);
}

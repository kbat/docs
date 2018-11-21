#include <iostream>
using std::cout;
using std::endl;

void printMe(int i) { cout << "I am an integer: " << i << endl; }
void printMe(int *i) { cout << "I am a pointer to an integer: " << i << endl; }

int main() {
  printMe(0);
//  printMe(NULL);      // error: call to 'printMe' is ambiguous
  printMe(nullptr);   // works fine
}

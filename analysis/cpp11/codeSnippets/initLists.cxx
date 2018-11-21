#include <iostream>
#include <vector>
using std::cout;
using std::endl;

class Position{
  public:
    Position() {fX = 0; fY = 0; fZ = 0;};
    Position(int x, int y, int z) {fX = x; fY = y; fZ = z;};
    ~Position(){};
    void printCoordinates(){cout << "(" << fX << ", " << fY << ", " << fZ << ")" << endl;};
  private:
    int fX;
    int fY;
    int fZ;
};

int main() {

  Position centreOfTheUniverse = {-1, -100, -1};
  centreOfTheUniverse.printCoordinates();

  std::vector<Position> places = {{1, 2, 3}, {}, {4, 5, 6}};
  for(auto i : places) i.printCoordinates();
}

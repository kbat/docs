# Initializer lists

Initialization lists are well known from arrays:

```cpp
int koordinates[3] = {1, 2, 3};
```
Since C++11 this intuitive concept can also be used to initialize class objects:

```cpp
class Position{
  public:
    Position() {fX = 0; fY = 0; fZ = 0;};
    Position(int x, int y, int z) {fX = x; fY = y; fZ = z;};
    ~Position(){};
  private:
    int fX;
    int fY;
    int fZ;
};
```

The following commands will call the respective constructors:

```cpp
Position centreOfTheUniverse = {};
Position endOfTheWorld = {-1, -1, -1};
```

With the help of initializer lists, defining vectors of objects becomes more readable and intuitive:

```cpp
std::vector<Position> nicePlaces = {{1, 2, 3}, {}, {5, 6, 7}};
```

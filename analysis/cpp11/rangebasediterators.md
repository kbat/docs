# Range-based iterations

Range-based for loops provide a simple and intuitive way to loop over iterable types like stl containers or C arrays.

```cpp
std::vector<int> v{1,4,5};
for (const auto& val : v) cout << val << endl;


float arr[] = {4.,5.,6.};
for (const auto& val : arr) cout << val << endl;

for (auto val : arr) val*=val;
for (const auto& val : arr) cout << val << endl;

for (auto& val : arr) val*=val;
for (const auto& val : arr) cout << val << endl;

```
Can also be used with ROOT containers (returns pointer to TObject!):
```cpp
TClonesArray arr("TParticle");
for (int i = 0; i < 10; ++i) {
  TParticle& part = *static_cast<TParticle*>(arr.ConstructedAt(i));
  part.SetPdgCode(i);
}
for (auto o : arr) {
  TParticle& part = *static_cast<TParticle*>(o);
  std::cout << "Pdg code: " <<  part.GetPdgCode() << '\n';
}
```

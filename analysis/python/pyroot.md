#PyROOT

many existing third-party applications and libraries have therefore so-called “Python bindings.” PyROOT provides Python bindings for ROOT: it enables cross-calls from ROOT/Cling into Python and vice versa, the intermingling of the two interpreters, and the transport of user-level objects from one interpreter to the other. PyROOT enables access from ROOT to any application or library that itself has Python bindings, and it makes all ROOT functionality directly available from the python interpreter.


The Python scripting language is widely used for scientific programming, including high performance and distributed parallel code (see http://www.scipy.org). It is the second most popular scripting language (after Perl) and enjoys a wide-spread use as a “glue language”: practically every library and application these days comes with Python bindings (and if not, they can be easily written or generated).

PyROOT, a Python extension module, provides the bindings for the ROOT class library in a generic way using the Cling dictionary. This way, it allows the use of any ROOT classes from the Python interpreter, and thus the “glue-ing” of ROOT libraries with any non-ROOT library or applications that provide Python bindings. Further, PyROOT can be loaded into the Cling interpreter to allow (as of now still rudimentary) access to Python classes. The best way to understand the benefits of PyROOT is through a few examples.

## Access to ROOT from Python

There are several tools for scientific analysis that come with bindings that allow the use of these tools from the Python interpreter. PyROOT provides this for users who want to do analysis in Python with ROOT classes. The following example shows how to fill and display a ROOT histogram while working in Python. Of course, any actual analysis code may come from somewhere else through other bindings, e.g. from a C++ program.

When run, the next example will display a 1-dimensional histogram showing a Gaussian distribution. More examples like the one above are distributed with ROOT under the $ROOTSYS/tutorials directory.

```python
from ROOT import gRandom,TCanvas,TH1F

c1 = TCanvas('c1','Example',200,10,700,500)
hpx = TH1F('hpx','px',100,-4,4)

for i in xrange(25000):
    px = gRandom.Gaus()
    hpx.Fill(px)

hpx.Draw()
c1.Update()
```

## Access to ROOT classes

Before a ROOT class can be used from Python, its dictionary needs to be loaded into the current process. Starting with ROOT version 4.00/06, this happens automatically for all classes that are declared to the auto-loading mechanism through so-called rootmap files. Effectively, this means that all classes in the ROOT distributions are directly available for import. For example:

```python
from ROOT import TCanvas          # available at startup
c = TCanvas()

from ROOT import TLorentzVector   # triggers auto-load of libPhysics
l = TLorentzVector()
```
Although it is not recommended, a simple way of working with PyROOT is doing a global import:

```python
from ROOT import *

c = TCanvas()
l = TLorentzVector()
```

Keeping the ROOT namespace (“import ROOT”), or only importing from ROOT those classes that you will actually use (see above), however, will always be cleaner and clearer:

```python
import ROOT

c = ROOT.TCanvas()
l = ROOT.TLorentzVector()
```
## Some simple examples

```python
from ROOT import TF1, TCanvas

class Linear:
    def __call__( self, x, par ):
        return par[0] + x[0]*par[1]

# create a linear function with offset 5, and pitch 2
f = TF1('pyf2',Linear(),-1.,1.,2)
f.SetParameters(5.,2.)

# plot the function
c = TCanvas()
f.Draw()
```

## Fitting a histogram

```python
from ROOT import TF1, TH1F, TCanvas

class Linear:
    def __call__( self, x, par ):
        return par[0] + x[0]*par[1]

# create a linear function for fitting
f = TF1('pyf3',Linear(),-1.,1.,2)

# create and fill a histogram
h = TH1F('h','test',100,-1.,1.)
f2 = TF1('cf2','6.+x*4.5',-1.,1.)
h.FillRandom('cf2',10000)

# fit the histo with the python 'linear' function
h.Fit(f)

# print results
par = f.GetParameters()
print('fit results: const =', par[0], ',pitch =', par[1])
```

## Working with trees
Next to making histograms, working with trees is probably the most common part of any analysis. The TTree implementation uses pointers and dedicated buffers to reduce the memory usage and to speed up access. Consequently, mapping TTree functionality to Python is not straightforward, and most of the following features are implemented in ROOT release 4.01/04 and later only, whereas you will need 5.02 if you require all of them.
19.1.9.1 Accessing an Existing Tree

Let us assume that you have a file containing TTrees, TChains, or TNtuples and want to read the contents for use in your analysis code. This is commonly the case when you work with the result of the reconstruction software of your experiment (e.g. the combined ntuple in ATLAS). The following example code outlines the main steps (you can run it on the result of the tree1.C macro):

```python
from ROOT import TFile

# open the file
myfile = TFile('tree1.root')

# retrieve the ntuple of interest
mychain = myfile.Get('t1')
entries = mychain.GetEntriesFast()

for jentry in xrange(entries):
    # get the next tree in the chain and verify
    ientry = mychain.LoadTree(jentry)
    if ientry < 0:
        break

    # copy next entry into memory and verify
    nb = mychain.GetEntry(jentry)
    if nb<=0:
        continue

    # use the values directly from the tree
    nEvent = int(mychain.ev)
    if nEvent<0:
        continue

    print(mychain.pz, '=', mychain.px*mychain.px, '+', mychain.py*mychain.py)
```

## Writing a tree

Writing a ROOT TTree in a Python session is a little convoluted, if only because you will need a C++ class to make sure that data members can be mapped, unless you are working with built-in types. Here is an example for working with the latter only:

``python
from ROOT import TFile, TTree
from array import array

h = TH1F('h1','test',100,-10.,10.)
f = TFile('test.root','recreate')
t = TTree('t1','tree with histos')
maxn = 10
n = array('i',[0])
d = array('f',maxn*[0.])
t.Branch('mynum',n,'mynum/I')
t.Branch('myval',d,'myval[mynum]/F')

for i in range(25):
    n[0] = min(i,maxn)
    for j in range(n[0]):
        d[j] = i*0.1+j
        t.Fill()

f.Write()
f.Close()
```
The use of arrays is needed, because the pointer to the address of the object that is used for filling must be given to the TTree::Branch() call, even though the formal argument is declared a ’void*'. In the case of ROOT objects, similar pointer manipulation is unnecessary, because the full type information is available, and TTree::Branch() has been Pythonized to take care of the call details. However, data members of such objects that are of built-in types, still require something extra since they are normally translated to Python primitive types on access and hence their address cannot be taken.

## Creating your own classes

A user’s own classes can be accessed after loading, either directly or indirectly, the library that contains the dictionary. One easy way of obtaining such a library, is by using ACLiC:

```cpp
$ cat MyClass.C
class MyClass {
public:

MyClass(int value = 0) {
m_value = value;
}

void SetValue(int value) {
m_value = value;
}

int GetValue() {
return m_value;
}

private:
    int m_value;
};

$ echo .L MyClass.C+ | root.exe -b
[...]
Info in <TUnixSystem::ACLiC>: creating shared library [..]/./MyClass_C.so
$
```
Then you can use it, for example, like so:

```python
from ROOT import gSystem

# load library with MyClass dictionary
gSystem.Load('MyClass_C')

# get MyClass from ROOT
from ROOT import MyClass
# use MyClass
m = MyClass(42)
print(m.GetValue())
```
You can also load a macro directly, but if you do not use ACLiC, you will be restricted to use the default constructor of your class, which is otherwise fully functional. For example:

```python
from ROOT import gROOT

# load MyClass definition macro (append '+' to use ACLiC)
gROOT.LoadMacro('MyClass.C')

# get MyClass from ROOT
from ROOT import MyClass

# use MyClass
m = MyClass()
m.SetValue(42)
print(m.GetValue())
```

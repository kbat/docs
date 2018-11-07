#PyROOT

In the previous sections, we have introduced some basics of Python to you. Now, it's time to make the connection between ROOT and Python, and see why Python can be pretty convenient. 

## Python bindings

Many existing third-party applications and libraries have so-called "Python bindings". 

{% callout "Bindings" %}
Binding generally refers to a mapping of one thing to another. In the context of software libraries, bindings are wrapper libraries that bridge two programming languages, so that a library written for one language can be used in another language.

Many software libraries are written in system programming languages such as C or C++. To use such libraries from another language, usually of higher-level, a binding to the library must be created in that language, possibly requiring recompiling the language's code, depending on the amount of modification needed. However, most languages offer a foreign function interface, such as Python's ctypes.
{% endcallout %}


PyROOT provides Python bindings for ROOT: it enables cross-calls from ROOT/Cling into Python and vice versa, the intermingling of the two interpreters, and the transport of user-level objects from one interpreter to the other. PyROOT enables access from ROOT to any application or library that itself has Python bindings, and it makes all ROOT functionality directly available from the python interpreter.


The Python scripting language is widely used for scientific programming, including high performance and distributed parallel code (see http://www.scipy.org). It is the second most popular scripting language (after Perl) and enjoys a wide-spread use as a “glue language”: practically every library and application these days comes with Python bindings (and if not, they can be easily written or generated).

PyROOT, a Python extension module, provides the bindings for the ROOT class library in a generic way using the Cling dictionary. This way, it allows the use of any ROOT classes from the Python interpreter, and thus the “glue-ing” of ROOT libraries with any non-ROOT library or applications that provide Python bindings. 

## Access to ROOT from Python

There are several tools for scientific analysis that come with bindings that allow the use of these tools from the Python interpreter. PyROOT provides this for users who want to do analysis in Python with ROOT classes. The following example shows how to fill and display a ROOT histogram while working in Python. Of course, any actual analysis code may come from somewhere else through other bindings, e.g. from a C++ program.

When run, the next example will display a 1-dimensional histogram showing a Gaussian distribution. 

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

{% callout "Make sure that PyROOT is enabled" %}
To make use of the Python Bindings of ROOT, make sure that the build of the PyROOT module is enabeled when you compile ROOT. If you use ROOT through aliBuild, ROOT6 is built with python support out-of-the-box. For ROOT5 based builds, PyROOT support is disabeled by default in aliBuild. For a manual build of ROOT, Python bindings are enabled by default, but can be disabled via the CMake options.

To load the ROOT module in your Python code, make sure that `libPyROOT.so` and the `ROOT.py` module can be resolved by the system by entering your (Ali)ROOT environment. 

{% endcallout %}


## Access to ROOT classes

Before a ROOT class can be used from Python, its dictionary needs to be loaded into the current process. This happens automatically for all classes that are declared to the auto-loading mechanism through so-called rootmap files. Effectively, this means that all classes in the ROOT distributions are directly available for import. For example:

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

You have seen in the previous section that we can resolve all ROOT classes through python bindings. 

{% challenge "Fitting a histogram" %}
Take the code example from the previous section as a starting point, and write a simple piece of python code that
* defines a linear function
* fills a histogram with some linearly distributed data
* fits the linear function to the histogram

{% solution "Click to check possible solution" %}

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
{% endchallenge %}

## Advanced examples: working with trees
Next to making histograms, working with trees is probably the most common part of any analysis. The TTree implementation uses pointers and dedicated buffers to reduce the memory usage and to speed up access. Consequently, mapping TTree functionality to Python is not straightforward.

Let us assume that you have a file containing TTrees, TChains, or TNtuples and want to read the contents for use in your analysis code. The following example code outlines the main steps :

{% challenge "But first ... create a tree" %}
To run the Python code below, you need a small tree
{% solution "Click here for a generic example which fills a suitable tree" %}
Place the following in a file called `tree1.C`
```cpp

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TBrowser.h"
#include "TH2.h"
#include "TRandom.h"

void tree1w()
{
   //create a Tree file tree1.root

   //create the file, the Tree and a few branches
   TFile f("tree1.root","recreate");
   TTree t1("t1","a simple Tree with simple variables");
   Float_t px, py, pz;
   Double_t random;
   Int_t ev;
   t1.Branch("px",&px,"px/F");
   t1.Branch("py",&py,"py/F");
   t1.Branch("pz",&pz,"pz/F");
   t1.Branch("random",&random,"random/D");
   t1.Branch("ev",&ev,"ev/I");

   //fill the tree
   for (Int_t i=0;i<10000;i++) {
     gRandom->Rannor(px,py);
     pz = px*px + py*py;
     random = gRandom->Rndm();
     ev = i;
     t1.Fill();
  }

  //save the Tree header. The file will be automatically closed
  //when going out of the function scope
  t1.Write();
}

void tree1r()
{
   //read the Tree generated by tree1w and fill two histograms

   //note that we use "new" to create the TFile and TTree objects !
   //because we want to keep these objects alive when we leave this function.
   TFile *f = new TFile("tree1.root");
   TTree *t1 = (TTree*)f->Get("t1");
   Float_t px, py, pz;
   Double_t random;
   Int_t ev;
   t1->SetBranchAddress("px",&px);
   t1->SetBranchAddress("py",&py);
   t1->SetBranchAddress("pz",&pz);
   t1->SetBranchAddress("random",&random);
   t1->SetBranchAddress("ev",&ev);

   //create two histograms
   TH1F *hpx   = new TH1F("hpx","px distribution",100,-3,3);
   TH2F *hpxpy = new TH2F("hpxpy","py vs px",30,-3,3,30,-3,3);

   //read all entries and fill the histograms
   Long64_t nentries = t1->GetEntries();
   for (Long64_t i=0;i<nentries;i++) {
     t1->GetEntry(i);
     hpx->Fill(px);
     hpxpy->Fill(px,py);
  }

  //we do not close the file. We want to keep the generated histograms
  //we open a browser and the TreeViewer
  if (gROOT->IsBatch()) return;
  new TBrowser();
  t1->StartViewer();
  // in the browser, click on "ROOT Files", then on "tree1.root".
  //     you can click on the histogram icons in the right panel to draw them.
  // in the TreeViewer, follow the instructions in the Help button.
}

void tree1() {
   tree1w();
   tree1r();
}
```
{% endchallenge %}

So, let's assume that you have cooked up a tree using the mini-code above, or you have a tree of your own. Below is a snippet of Python code that will read info from the branches of your tree and prints some extracted information to the screen. If you are interested, you can try to visualize the data in histograms. 

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

```python
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
The use of arrays is needed, because the pointer to the address of the object that is used for filling must be given to the TTree::Branch() call, even though the formal argument is declared a ’void\*'. In the case of ROOT objects, similar pointer manipulation is unnecessary, because the full type information is available, and TTree::Branch() has been Pythonized to take care of the call details. However, data members of such objects that are of built-in types, still require something extra since they are normally translated to Python primitive types on access and hence their address cannot be taken.

## Creating your own classes

A user’s own classes can be accessed after loading, either directly or indirectly, the library that contains the dictionary. One easy way of obtaining such a library, is by using ACLiC. Let's first make up a very small class, by putting in a file `MyClass.C` the following code

```cpp
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
```

Create the shared library using ACLiC:

```
root -e 'gInterpreter->LoadMacro("MyClass.C++o");' -q
```

{% challenge "Optimization or debug symbols ?" %}
Anyone have any idea what the difference is between 
```
root -e 'gInterpreter->LoadMacro("MyClass.C++o");' -q
```
and
```
root -e 'gInterpreter->LoadMacro("MyClass.C++g");' -q
```
{% solution "Drum roll" %}
Using `o`, we specify to the compiler to use optimization for speed. Using `g`, we specify to the compiler that we want to enable debug symbols. Confused by this? Check out Part 9 of this tutorial, 'Best Practices', where there is a lot of information about this!
{% endchallenge %}

We have now compiled our class. If we want to access it with Python, we can e.g. do

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

# Parallallism in ROOT

By now, we have seen that multi threading (and also multi processing) is powerful but quite complicated. Our analyses are often particularity well-suited for multi threading though: we often repeat independent tasks almost indefinitely. Luckily, ROOT provides us with a simple interface to perform fast and safe multi threading and multi processing. 

The `TThread` class has been developed to provide a platform independent interface to threads for ROOT. However, ROOT6 provides us with many ways to easily access *implicit* parallellism, which makes our lives much easier than defining `POSIX`-like threads by hand.

## Implicit multi threading

The easiest way to learn about ROOT's implicit multi threading, is to look at an example. Say we want to read some information from a tree. This is a process that can very easily be multi threaded: operations on branches (like I/O, decompression, etc) are **independent** and can be carried out concurrently. 

To tell ROOT6 to operate on our tree in a multithreaded way, all we have to do is enable implicit multithreading:

```cpp
   ROOT::EnableImplicitMT(nthreads);
```

Let's look at this in a real world example. 

```cpp
   // First enable implicit multi-threading globally, so that the implicit parallelisation is on.
   // The parameter of the call specifies the number of threads to use.
   int nthreads = 4;
   ROOT::EnableImplicitMT(nthreads);
   // Open the file containing the tree
   auto file = TFile::Open("http://root.cern.ch/files/h1/dstarmb.root");
   // Get the tree
   TTree *tree = nullptr;
   file->GetObject<TTree>("h42", tree);
   const auto nEntries = tree->GetEntries();
   // Read the branches in parallel.
   // Note that the interface does not change, the parallelisation is internal
   for (auto i : ROOT::TSeqUL(nEntries)) {
      tree->GetEntry(i); // parallel read
   }
   // IMT parallelisation can be disabled for a specific tree
   tree->SetImplicitMT(false);
   // If now GetEntry is invoked on the tree, the reading is sequential
   for (auto i : ROOT::TSeqUL(nEntries)) {
      tree->GetEntry(i); // sequential read
   }
   // Parallel reading can be re-enabled
   tree->SetImplicitMT(true);
   // IMT can be also disabled globally.
   // As a result, no tree will run GetEntry in parallel
   ROOT::DisableImplicitMT();
   // This is still sequential: the global flag is disabled, even if the
   // flag for this particular tree is enabled
   for (auto i : ROOT::TSeqUL(nEntries)) {
      tree->GetEntry(i); // sequential read
   }
```

{% callout "Exercise" %}
Run the above example using a single-threaded or multi threaded approach. Time how quickly the tree is read (use e.g. the `TStopWatch` class), using different numbers of threads. 
{% endcallout %}

## TProcessExecutor

Our first example was very easy - we did not have to do anything except for telling ROOT to enable multi threading. However, we have fancier tools to use, namely the `TProcessExecutor`. 

The `TProcessExecutor` uses multi processing (as the name implies). It provides a simple interface to execute the same task multiple times in parallel, possibly with different arguments every time (this mimics the behaviour of python's pool.Map method). 

This class inherits its interfaces from ROOT::`TExecutor`. To use it, we have to identify a **pool** of workers, which we provide with a task, in the form of a lambda expression, via a **Map** method:

```cpp
// define a pool of n workers
ROOT::TProcessExecutor pool(n); 
// define what the workers in the pool have to do
auto result = pool.Map([](lamda arguments) { function }, map arguments);
```

The two possible usages of the Map method are:

*    `Map(F func, unsigned nTimes)`: func is executed nTimes with no arguments
*    `Map(F func, T& args)`: func is executed on each element of the collection of arguments `args`

For either signature, `func` is executed as many times as needed by a pool of `n` workers; the number of workers can be passed to the constructor or set via `SetNWorkers`. It defaults to the number of cores. A collection containing the result of each execution is returned.

{% callout "Beware" %}
Note: the user is responsible for the deletion of any object that might be created upon execution of `func`, returned objects included: `ROOT::TProcessExecutor` never deletes what it returns, it simply forgets it.
Note: that the usage of `ROOT::TProcessExecutor::Map` is indicated only when the task to be executed takes more than a few seconds, otherwise the overhead introduced by `Map` will outrun the benefits of parallel execution on most machines.

{% endcallout %}

Let's take a closer look at how we can construct our processes. What we need are the following parameters
*    `func`	a lambda expression, an std::function, a loaded macro, a functor class or a function that takes zero arguments (for the first signature) or one (for the second signature).
*    `args`	a standard vector, a `ROOT::TSeq` of integer type or an initializer list for the second signature. 

For example, using a lambda expression and an initializer list for the `ROOT::TSeq`, our process executor could look like

```cpp
// define a pool of two workers
ROOT::TProcessExecutor pool(2);
// define our method: it will simply return the square of each element of a vector 
auto squares = pool.Map([](int a) { return a*a; }, {1,2,3});
```

This looks more complicated than it is. Let's look at an example in the wild, where we fill four histograms with random numbers simultaneously:

```cpp
// Total amount of numbers
const UInt_t nNumbers = 20000000U;
// The number of workers
const UInt_t nWorkers = 4U;

   // We define our work item
   auto workItem = [](UInt_t workerID) {
      // One generator, file and ntuple per worker
      TRandom3 workerRndm(workerID); // Change the seed
      TFile f(Form("myFile_%u.root", workerID), "RECREATE");
      TH1F h(Form("myHisto_%u", workerID), "The Histogram", 64, -4, 4);
      for (UInt_t i = 0; i < nNumbers; ++i) {
         h.Fill(workerRndm.Gaus());
      }
      h.Write();
      return 0;
   };

   // Create the pool of workers
   ROOT::TProcessExecutor workers(nWorkers);
   // Fill the pool with work
   workers.Map(workItem, ROOT::TSeqI(nWorkers));
```

{% callout "Exercise" %}

First, run the example that is shown above. Then, think of a macro that you have somewhere on your laptop, and see if you can multi process it. 

{% endcallout %}

## TTreeProcessorMT

A lot of the work that we do, involves manipulation trees. The `TTreeProcessorMT` method provides an implicit parallelisation of 
- the reading and 
- processing 

of a `TTree`. 

In particular, when invoking `Process`, the user provides a function that iterates on a subrange of the tree via a `TTreeReader`. Multiple tasks will be spawned, one for each sub-range, so that the processing of the tree is parallelised.

### Thread safety

Since two invocations of the user function can potentially run in parallel, the function code must be **thread safe** (remember the discussion on race conditions and deadlocks). For this, we can use, `ROOT::TThreadedObject`, which makes objects thread private. With the help of this class, histograms can be filled safely inside the user function and then merged at the end to get the final result.

By means of its `Process` method, `ROOT::TTreeProcessorMT` provides a way to process the entries of a `TTree` in parallel. When invoking `TTreeProcessor::Process`, the user passes a function whose only parameter is a `TTreeReader`. The function iterates on a subrange of entries by using that `TTreeReader`.

The implementation of `ROOT::TTreeProcessorMT` parallelizes the processing of the subranges, each corresponding to a cluster in the `TTree`. This is possible thanks to the use of a `ROOT::TThreadedObject`, so that each thread works with its own `TFile` and `TTree` objects. 

Yet again, let's give it a whirl!

```cpp
   // First enable implicit multi-threading globally, so that the implicit parallelisation is on.
   // The parameter of the call specifies the number of threads to use.
   int nthreads = 4;
   ROOT::EnableImplicitMT(nthreads);
   // Create one TThreadedObject per histogram to fill during the processing of the tree
   ROOT::TThreadedObject<TH1F> ptHist("pt_dist", "p_{T} Distribution;p_{T};dN/p_{T}dp_{T}", 100, 0, 5);
   ROOT::TThreadedObject<TH1F> pzHist("pz_dist", "p_{Z} Distribution;p_{Z};dN/dp_{Z}", 100, 0, 5);
   ROOT::TThreadedObject<TH2F> pxpyHist("px_py", "p_{X} vs p_{Y} Distribution;p_{X};p_{Y}", 100, -5., 5., 100, -5., 5.);
   // Create a TTreeProcessorMT: specify the file and the tree in it
   ROOT::TTreeProcessorMT tp("http://root.cern.ch/files/tp_process_imt.root", "events");
   // Define the function that will process a subrange of the tree.
   // The function must receive only one parameter, a TTreeReader,
   // and it must be thread safe. To enforce the latter requirement,
   // TThreadedObject histograms will be used.
   auto myFunction = [&](TTreeReader &myReader) {
      TTreeReaderValue<std::vector<ROOT::Math::PxPyPzEVector>> tracksRV(myReader, "tracks");
      // For performance reasons, a copy of the pointer associated to this thread on the
      // stack is used
      auto myPtHist = ptHist.Get();
      auto myPzHist = pzHist.Get();
      auto myPxPyHist = pxpyHist.Get();
      while (myReader.Next()) {
         auto tracks = *tracksRV;
         for (auto &&track : tracks) {
            myPtHist->Fill(track.Pt(), 1. / track.Pt());
            myPxPyHist->Fill(track.Px(), track.Py());
            myPzHist->Fill(track.Pz());
         }
      }
   };
   // Launch the parallel processing of the tree
   tp.Process(myFunction);
   // Use the TThreadedObject::Merge method to merge the thread private histograms
   // into the final result
   auto ptHistMerged = ptHist.Merge();
   auto pzHistMerged = pzHist.Merge();
   auto pxpyHistMerged = pxpyHist.Merge();
```
{% callout "Exercise" %}
The exercise here is the same as before: run the code snippet above, and play with the number of threads. You can of course plot the histograms by simply doing
```
ptHistMerged->DrawCopy()
```
{% endcallout %}



## Multi core reading of TChain data

If time permits, let's look at a few more things. Below, it is illustrated how we can use a `TTreeProcessorMP` (so a multi **processed** version of the `TTreeReader` to delegate reading of TChain data to multiple cores


```cpp
   // No nuisance for batch execution
   gROOT->SetBatch();
   //---------------------------------------
   // Perform the operation sequentially
   TChain inputChain("multiCore");
   inputChain.Add("mp101_multiCore_*.root");
   if (inputChain.GetNtrees() <= 0) {
      Printf(" No files in the TChain: did you run mp101_fillNtuples.C before?");
      return 1;
   }
   TH1F outHisto("outHisto", "Random Numbers", 128, -4, 4);
   inputChain.Draw("r >> outHisto");
   outHisto.Fit("gaus");
   //---------------------------------------
   // We now go MP!
   // TProcessExecutor offers an interface to directly process trees and chains without
   // the need for the user to go through the low level implementation of a
   // map-reduce.
   // We adapt our parallelisation to the number of input files
   const auto nFiles = inputChain.GetListOfFiles()->GetEntries();
   // This is the function invoked during the processing of the trees.
   auto workItem = [](TTreeReader &reader) {
      TTreeReaderValue<Float_t> randomRV(reader, "r");
      auto partialHisto = new TH1F("outHistoMP", "Random Numbers", 128, -4, 4);
      while (reader.Next()) {
         partialHisto->Fill(*randomRV);
      }
      return partialHisto;
   };
   // Create the pool of processes
   ROOT::TTreeProcessorMP workers(nFiles);
   // Process the TChain
   auto sumHistogram = workers.Process(inputChain, workItem, "multiCore");
   sumHistogram->Fit("gaus", 0);
```
{% challenge "Exercise" %}
Run the above code snippet. Note, that it needs some input! This can be generated by the mp101_fillNtuples.C macro, which is given below. 
{% solution "generate your trees..." %}
```cpp
// Total amount of numbers
const UInt_t nNumbers = 20000000U;

// The number of workers
const UInt_t nWorkers = 4U;

// We split the work in equal parts
const auto workSize = nNumbers / nWorkers;

// A simple function to fill ntuples randomly
void fillRandom(TNtuple &ntuple, TRandom3 &rndm, UInt_t n)
{
   for (auto i : ROOT::TSeqI(n))
      ntuple.Fill(rndm.Gaus());
}

Int_t mp101_fillNtuples()
{

   // No nuisance for batch execution
   gROOT->SetBatch();

   //---------------------------------------
   // Perform the operation sequentially

   // Create a random generator and and Ntuple to hold the numbers
   TRandom3 rndm(1);
   TFile ofile("mp101_singleCore.root", "RECREATE");
   TNtuple randomNumbers("singleCore", "Random Numbers", "r");
   fillRandom(randomNumbers, rndm, nNumbers);
   randomNumbers.Write();
   ofile.Close();

   //---------------------------------------
   // We now go MP!

   // We define our work item
   auto workItem = [](UInt_t workerID) {
      // One generator, file and ntuple per worker
      TRandom3 workerRndm(workerID); // Change the seed
      TFile ofile(Form("mp101_multiCore_%u.root", workerID), "RECREATE");
      TNtuple workerRandomNumbers("multiCore", "Random Numbers", "r");
      fillRandom(workerRandomNumbers, workerRndm, workSize);
      workerRandomNumbers.Write();
      return 0;
   };

   // Create the pool of workers
   ROOT::TProcessExecutor workers(nWorkers);

   // Fill the pool with work
   workers.Map(workItem, ROOT::TSeqI(nWorkers));

   return 0;
}
```
{% endchallenge %}


## TTaskGroup

The last item that we will illustrate, is the `TTaskGroup`. TTask is a base class that can be used to build a complex tree of Tasks. Each TTask derived class may contain other TTasks that can be executed recursively, such that a complex program can be dynamically built and executed by invoking the services of the top level Task or one of its subtasks. 
{% callout "TTasks in ALICE" %}
If you do analysis in ALICE, you surely know `TTasks`, since they form the base class of `AliAnalysisTaskSE`. 
{% endcallout %}

`TTaskGroup` provides a way to manage the asynchronous execution of work items. A TTaskGroup represents concurrent execution of a group of tasks. Tasks may be dynamically added to the group as it is executing. 

Below an example of usage of the `TTaskGroup` for calculating Fibonacci series. See if you can run. And who knows, maybe in a few years, your analysis will benefit from paralellism as well ...

```cpp
int Fibonacci(int n)
{
   if (n < 2) {
      return n;
   } else {
      int x, y;
      ROOT::Experimental::TTaskGroup tg;
      tg.Run([&] { x = Fibonacci(n - 1); });
      tg.Run([&] { y = Fibonacci(n - 2); });
      tg.Wait();
      return x + y;
   }
}
void mt302_TTaskGroupNested()
{
   ROOT::EnableImplicitMT(4);
   cout << "Fibonacci(33) = " << Fibonacci(33) << endl;
}
```

You will have to store the above example in a macro, let's call it fib.C, and load it in a ROOT session to execute it:

```
root .L fib.C 
root [1] mt302_TTaskGroupNested()
Fibonacci(33) = 3524578
```

{% callout "ROOT::Experimental" %}
You might notice, that in the code snippet above, we use items from the `ROOT::Experimental` namespace. As the name implies, these are experimental features. Depending on your ROOT6 version, these features might have been moved from the `Experimental` namespace to a general namespace, in which case, the `T` that starts the class name, might have been changed into an `R` to avoid namespace clashes. 
{% endcallout %}

## Enabling thread safety

In the previous section, we have talked quite a bit about race conditions and thread safety. How are these taken care of in the ROOT landscape? 

ROOT provides a global mutex to make ROOT thread safe/aware. By enabling it, the following processes are ensured to be safe
*    concurrent construction and destruction of TObjects, including the ones registered in ROOT's global lists (e.g. gROOT->GetListOfCleanups(), gROOT->GetListOfFiles())
*    concurrent usage of different ROOT objects from different threads, including ones with global state (e.g. TFile, TTree, TChain) with the exception of graphics classes (e.g. TCanvas)
*    concurrent calls to ROOT's type system classes, e.g. TClass and TEnum
*    concurrent calls to the interpreter through gInterpreter
*    concurrent loading of ROOT plug-ins

In addition, gDirectory, gFile and gPad become a thread-local variable. In all threads, gDirectory defaults to gROOT, a singleton which supports thread-safe insertion and deletion of contents. gFile and gPad default to nullptr, as it is for single-thread programs.

Note that there is no DisableThreadSafety(). ROOT's thread-safety features cannot be disabled once activated. 

Below is an example of parallell execution in ROOT, guaranteeing thread safety:


```cpp
const UInt_t poolSize = 4U;
Int_t mtbb201_parallelHistoFill()
{
   ROOT::EnableThreadSafety();
   TH1::AddDirectory(false);
   ROOT::TThreadExecutor pool(poolSize);
   auto fillRandomHisto = [](int seed = 0) {
      TRandom3 rndm(seed);
      auto h = new TH1F("myHist", "Filled in parallel", 128, -8, 8);
      for (auto i : ROOT::TSeqI(1000000)) {
         h->Fill(rndm.Gaus(0, 1));
      }
      return h;
   };
   auto seeds = ROOT::TSeqI(23);
   ROOT::ExecutorUtils::ReduceObjects<TH1F *> redfunc;
   auto sumRandomHisto = pool.MapReduce(fillRandomHisto, seeds, redfunc);
   auto c = new TCanvas();
   sumRandomHisto->Draw();
   return 0;
}
```

{% callout "General exercises" %}
As already mentioned in the text, exercises here comprise running the examples, and looking for good candidates in your own macros that you could re-write in a multi threaded or multi processed way. 

It is always good to test whether or not multi threading / processing actually helps: both come with overhead, and depending on the task at hand, can either speed up execution or your program, slow it down, or do nothing at all. 
{% endcallout %}

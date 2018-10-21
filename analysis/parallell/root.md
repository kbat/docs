# Threads with ROOT

A thread is an independent flow of control that operates within the same address space as other independent flows of controls within a process. In most UNIX systems, thread and process characteristics are grouped into a single entity called a process. Sometimes, threads are called “lightweight processes’’.

## Threads and Processes

In traditional single-threaded process systems, a process has a set of properties. In multi-threaded systems, these properties are divided between processes and threads.

{% callout "Process properties" %}

A process in a multi-threaded system is the changeable entity. It must be considered as an execution frame. It has all traditional process attributes, such as:

*    Process ID, process group ID, user ID, and group ID

*    Environment

*    Working directory

A process also provides a common address space and common system resources:

*    File descriptors

*    Signal actions

*    Shared libraries

*    Inter-process communication tools (such as message queues, pipes, semaphores, or shared memory)

{% endcallout %}

{% callout "Thread properties" %} 

A thread is the schedulable entity. It has only those properties that are required to ensure its independent flow of control. These include the following properties:

*    Stack

*    Scheduling properties (such as policy or priority)

*    Set of pending and blocked signals

*    Some thread-specific data (TSD)
{% endcallout %}

An example of thread-specific data is the error indicator, errno. In multi-threaded systems, errno is no longer a global variable, but usually a subroutine returning a thread-specific errno value. Some other systems may provide other implementations of errno. With respect to ROOT, a thread specific data is for example the gPad pointer, which is treated in a different way, whether it is accessed from any thread or the main thread.

Threads within a process must not be considered as a group of processes (even though in Linux each thread receives an own process id, so that it can be scheduled by the kernel scheduler). All threads share the same address space. This means that two pointers having the same value in two threads refer to the same data. Also, if any thread changes one of the shared system resources, all threads within the process are affected. For example, if a thread closes a file, the file is closed for all threads.

## The Initial Thread

When a process is created, one thread is automatically created. This thread is called the initial thread or the main thread. The initial thread executes the main routine in multi-threaded programs.

Note: At the end of this chapter is a glossary of thread specific terms

# Parallallism in ROOT



The `TThread` class has been developed to provide a platform independent interface to threads for ROOT. However, ROOT6 provides us with many ways to easily access *implicit* parallellism, which makes our lives much easier than defining `posix` threads by hand.


## Implicit multi threading

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
## TProcessExecutor

This class provides a simple interface to execute the same task multiple times in parallel, possibly with different arguments every time. This mimics the behaviour of python's pool.Map method.
ROOT::TProcessExecutor::Map

This class inherits its interfaces from ROOT::TExecutor
. The two possible usages of the Map method are:

*    Map(F func, unsigned nTimes): func is executed nTimes with no arguments
*    Map(F func, T& args): func is executed on each element of the collection of arguments args

For either signature, func is executed as many times as needed by a pool of fNWorkers workers; the number of workers can be passed to the constructor or set via SetNWorkers. It defaults to the number of cores.
A collection containing the result of each execution is returned.
{% callout "Beware" %}
Note: the user is responsible for the deletion of any object that might be created upon execution of func, returned objects included: ROOT::TProcessExecutor never deletes what it returns, it simply forgets it.
Note: that the usage of ROOT::TProcessExecutor::Map is indicated only when the task to be executed takes more than a few seconds, otherwise the overhead introduced by Map will outrun the benefits of parallel execution on most machines.

{% endcallout %}



Parameters
*    func	a lambda expression, an std::function, a loaded macro, a functor class or a function that takes zero arguments (for the first signature) or one (for the second signature).
    args	a standard vector, a ROOT::TSeq of integer type or an initializer list for the second signature. An integer only for the first.

Note: in cases where the function to be executed takes more than zero/one argument but all are fixed except zero/one, the function can be wrapped in a lambda or via std::bind to give it the right signature.
Note: the user should take care of initializing random seeds differently in each process (e.g. using the process id in the seed). Otherwise several parallel executions might generate the same sequence of pseudo-random numbers.
Return value:

An std::vector. The elements in the container will be the objects returned by func.

```cpp
ROOT::TProcessExecutor pool(2); auto squares = pool.Map([](int a) { return a*a; }, {1,2,3});
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

## TTreeProcessorMT

ethod provides an implicit parallelisation of the reading and processing of a TTree. In particular, when invoking Process, the user provides a function that iterates on a subrange of the tree via a TTreeReader. Multiple tasks will be spawned, one for each sub-range, so that the processing of the tree is parallelised. Since two invocations of the user function can potentially run in parallel, the function code must be thread safe. The example also introduces a new class, ROOT::TThreadedObject, which makes objects thread private. With the help of this class, histograms can be filled safely inside the user function and then merged at the end to get the final result.

By means of its Process method, ROOT::TTreeProcessorMT provides a way to process the entries of a TTree in parallel. When invoking TTreeProcessor::Process, the user passes a function whose only parameter is a TTreeReader. The function iterates on a subrange of entries by using that TTreeReader.

The implementation of ROOT::TTreeProcessorMT parallelizes the processing of the subranges, each corresponding to a cluster in the TTree. This is possible thanks to the use of a ROOT::TThreadedObject, so that each thread works with its own TFile and TTree objects. 

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

## Multi core reading of TChain data

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
## TTaskGroup

Calculate Fibonacci numbers exploiting nested parallellism through TTaskGroup
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
## Enabling thread safety

Enables the global mutex to make ROOT thread safe/aware.

The following becomes safe:

*    concurrent construction and destruction of TObjects, including the ones registered in ROOT's global lists (e.g. gROOT->GetListOfCleanups(), gROOT->GetListOfFiles())
*    concurrent usage of different ROOT objects from different threads, including ones with global state (e.g. TFile, TTree, TChain) with the exception of graphics classes (e.g. TCanvas)
*    concurrent calls to ROOT's type system classes, e.g. TClass and TEnum
*    concurrent calls to the interpreter through gInterpreter
*    concurrent loading of ROOT plug-ins

In addition, gDirectory, gFile and gPad become a thread-local variable. In all threads, gDirectory defaults to gROOT, a singleton which supports thread-safe insertion and deletion of contents. gFile and gPad default to nullptr, as it is for single-thread programs.

Note that there is no DisableThreadSafety(). ROOT's thread-safety features cannot be disabled once activated. 



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

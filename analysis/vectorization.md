# Vectorization

Vectorization is a topic that we will cover only very briefly, mostly to make you aware of its existence. 

## What is vectorization? 

Vectorization is a special case of automatic parallelization, where a computer program is converted from a scalar implementation, which processes a single pair of operands at a time, to a vector implementation, which processes one operation on multiple pairs of operands at once. For example, modern conventional computers, including specialized supercomputers, typically have vector operations that simultaneously perform operations such as the following four additions (via SIMD or SPMD hardware): 

An example would be a program to multiply two vectors of numeric data. A scalar approach would be something like: 

```cpp
 for (i = 0; i < 1024; i++)
    C[i] = A[i]*B[i];
```

This could be vectorized to look something like: 

```cpp
  for (i = 0; i < 1024; i+=4)
     C[i:i+3] = A[i:i+3]*B[i:i+3];
```

Here, C[i:i+3] represents the four array elements from C[i] to C[i+3] and the vector processor can perform four operations for a single vector instruction. Since the four vector operations complete in roughly the same time as one scalar instruction, the vector approach can run up to four times faster than the original code.

## How is this helpful ? 

Single instruction, multiple data (SIMD) is a class of parallel computers in Flynn's taxonomy. It describes computers with multiple processing elements that perform the same operation on multiple data points simultaneously. Such machines exploit data level parallelism, but not concurrency: there are simultaneous (parallel) computations, but only a single process (instruction) at a given moment. SIMD is particularly applicable to common tasks such as adjusting the contrast in a digital image or adjusting the volume of digital audio. Most modern CPU designs include SIMD instructions to improve the performance of multimedia use. Not to be confused with SIMT which utilizes threads. 

### Advantages

An application that may take advantage of SIMD is one where the same value is being added to (or subtracted from) a large number of data points, a common operation in many multimedia applications. One example would be changing the brightness of an image. Each pixel of an image consists of three values for the brightness of the red (R), green (G) and blue (B) portions of the color. To change the brightness, the R, G and B values are read from memory, a value is added to (or subtracted from) them, and the resulting values are written back out to memory.

With a SIMD processor there are two improvements to this process. For one the data is understood to be in blocks, and a number of values can be loaded all at once. Instead of a series of instructions saying "retrieve this pixel, now retrieve the next pixel", a SIMD processor will have a single instruction that effectively says "retrieve n pixels" (where n is a number that varies from design to design). For a variety of reasons, this can take much less time than retrieving each pixel individually, as with traditional CPU design.

Another advantage is that the instruction operates on all loaded data in a single operation. In other words, if the SIMD system works by loading up eight data points at once, the add operation being applied to the data will happen to all eight values at the same time. This parallelism is separate from the parallelism provided by a superscalar processor; the eight values are processed in parallel even on a non-superscalar processor, and a superscalar processor may be able to perform multiple SIMD operations in parallel. 

### Disadvantages

Not all algorithms can be vectorized easily. For example, a flow-control-heavy task like code parsing may not easily benefit from SIMD; however, it is theoretically possible to vectorize comparisons and "batch flow" to target maximal cache optimality, though this technique will require more intermediate state. Note: Batch-pipeline systems (example: GPUs or software rasterization pipelines) are most advantageous for cache control when implemented with SIMD intrinsics, but they are not exclusive to SIMD features. Further complexity may be apparent to avoid dependence within series such as code strings; while independence is required for vectorization.
Large register files which increases power consumption and require chip area.
Currently, implementing an algorithm with SIMD instructions usually requires human labor; most compilers don't generate SIMD instructions from a typical C program, for instance. Automatic vectorization in compilers is an active area of computer science research. (Compare vector processing.)

## Hardware
Small-scale (64 or 128 bits) SIMD became popular on general-purpose CPUs in the early 1990s and continued through 1997 and later with Motion Video Instructions (MVI) for Alpha. SIMD instructions can be found, to one degree or another, on most CPUs, including the IBM's AltiVec and SPE for PowerPC, HP's PA-RISC Multimedia Acceleration eXtensions (MAX), Intel's MMX and iwMMXt, SSE, SSE2, SSE3 SSSE3 and SSE4.x, AMD's 3DNow!, ARC's ARC Video subsystem, SPARC's VIS and VIS2, Sun's MAJC, ARM's NEON technology, MIPS' MDMX (MaDMaX) and MIPS-3D. The IBM, Sony, Toshiba co-developed Cell Processor's SPU's instruction set is heavily SIMD based. NXP founded by Philips developed several SIMD processors named Xetal. The Xetal has 320 16bit processor elements especially designed for vision tasks.

Modern graphics processing units (GPUs) are often wide SIMD implementations, capable of branches, loads, and stores on 128 or 256 bits at a time.

Intel's AVX SIMD instructions now process 256 bits of data at once. Intel's Larrabee prototype microarchitecture includes more than two 512-bit SIMD registers on each of its cores (VPU: Wide Vector Processing Units), and this 512-bit SIMD capability is being continued in Intel's Many Integrated Core Architecture (Intel MIC) and Skylake-X. 

# Vectorization support in ROOT

* Integration of VecCore in ROOT as the common vector abstraction in HEP
  * Definition of new ROOT SIMD types: ROOT::Double_v, ROOT::Float_v.
* Adaptation of TF1 to evaluate functions evaluating over vector types.
* Adaptation of the fitting classes and interfaces in ROOT to accept the new
* SIMD types and functions implementing them.
* Parallelization of the fitting objective functions (Max. Likelihood, Least
Squares)

TFormula supports vectorization. All the TF1 objected created with a formula expression can have a vectorized signature using ROOT::Double_v: TF1::EvalPar( ROOT::Double_v * x, double * p). The vectorization can then be used to speed-up fitting. It is not enabled by default, but it can be enabled by callig TF1::SetVectorized(true) or using the "VEC" option in the constructor of TF1, when ROOT has been built with VecCore and one vectorization library such as Vc.

```cpp
//Higgs Fit: Implementation of the scalar function
double func(const double *data, const double *params)
{
return params[0] * exp(-(*data + (-130.)) * (*data + (-130.)) / 2) +
params[1] * exp(-(params[2] * (*data * (0.01)) - params[3] *
((*data) * (0.01)) * ((*data) * (0.01))));
}
TF1 *f = new TF1(”fScalar”, func, 100, 200, 4);
f->SetParameters(1, 1000, 7.5, 1.5);
TH1D h1f(”h1f”, ”Test random numbers”, 12800, 100, 200);
h1f.FillRandom(”fvScalar”, 1000000);
h1f.Fit(f);
```

in the vectorized approach

```cpp
//Higgs Fit: Implementation of the vectorized function
ROOT::Double_v func(const ROOT::Double_v *data, const double *params)
{
return params[0] * exp(-(*data + (-130.)) * (*data + (-130.)) / 2) +
params[1] * exp(-(params[2] * (*data * (0.01)) - params[3] *
((*data) * (0.01)) * ((*data) * (0.01))));
}
//This code is totally backwards compatible
TF1 *f = new TF1(”fvCore”, func, 100, 200, 4);
f->SetParameters(1, 1000, 7.5, 1.5);
TH1D h1f(”h1f”, ”Test random numbers”, 12800, 100, 200);
h1f.FillRandom(”fvCore”, 1000000);
//Added multithreaded fit option
h1f.Fit(f, ”MULTITHREAD”);
```

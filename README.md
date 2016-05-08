# FAST ANDROID

FastAndroid is a android library written in C. It make use of The ARM® NEON™, a general purpose SIMD engine to process the array and matrix.

We create two neon-based libraries for android developers.

1. Matrix Library called FaMatrix. The library contains most useful matrix operations, including matrix transpose, addition, and multiplication. The library support short, int and float data types.
2. Basic Tools Library called FaCollection. This is a basic library and provide three basic tools, including vectorized calculation, sort and fast fourier transform.

More information can be found in our ["website"](http://chaoyali.github.io/parallel/)


## Implementation

We build an environment for android NDK developing. All C/NEON code can be found in the ["this address"](https://github.com/chaoyali/parallel/tree/master/neoncore/src/main/jni)


## Performance

Speed: The matrix multiplication achieves 14.6x speedup to Java version, 4.1x to C version multiplication. The transpose can achieve 8.9x to Java version, and 1.5x to C version. The Tools of calculating points variables can run as fast as 20x than serial code in C. The Tools of sorting array can run 5x - 7x faster than java code and 1.1x - 1.3x faster than quicksort code in C. Tools of fft can run 15x - 30x faster than Java implementation and 1.2x - 1.5x faster than same implementation on C code.

Energy Consumption: Device broken, so we should to fix it first. Coming soon.

## Online Document

Coming Soon...

## How to Import it

We generate .aar file in ["this address"](https://github.com/chaoyali/parallel/tree/master/neoncore/aar).

You can simple download it and imported it into Android Studio project. You can use it according to the documents. It can also be used in non-NEON device because it can make use of serial C code if the NEON code is unavailable.

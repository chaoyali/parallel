# FAST ANDROID

FastAndroid is a android library written in C. It make use of The ARM® NEON™, a general purpose SIMD engine to process the array and matrix.

We create two neon-based libraries for android developers.

1. Matrix Library called FaMatrix. The library contains most useful matrix operations, including matrix transpose, addition, and multiplication. The library support short, int and float data types.
2. Basic Tools Library called FaCollection. This is a basic library and provide three basic tools, including vectorized calculation, sort and fast fourier transform.

More information can be found in our [website](http://chaoyali.github.io/parallel/)


## Implementation

We build an environment for android NDK developing. All C/NEON code can be found in the [this address](https://github.com/chaoyali/parallel/tree/master/neoncore/src/main/jni)


## Contribution

In FaMatrix library, the matrix multiplication achieves 14.6x speedup to Java version, 4.1x to C version multiplication. The transpose can achieve 8.9x to Java version, and 1.5x to C version.

In FaCollection, the calculation of points variables can run as fast as 2.2x than serial code in C, the sorting algorithm can run 5x - 7x faster than official java code and 2.2x faster than comsort code in c, and 1.1x - 1.3x faster than quicksort code in C, FFT can run 3x - 5x faster than Java implementation and 1.2x - 1.5x faster than same implementation on C code.

Besides of the speedup of several useful algorithms, we also re-organize the data flow of FFT and make it better for data locality and can be use for SIMD easily. As far as we know, we do not have an implementation like it so far.

## API Document

We write a document to describe our api for users at [this address](https://github.com/chaoyali/parallel/tree/master/neoncore/docs).

## How to Import it

We generate .aar file in [this address](https://github.com/chaoyali/parallel/tree/master/neoncore/aar).

You can simple download it and imported it into Android Studio project. You can use it according to the documents. It can also be used in non-NEON device because it can make use of serial C code if the NEON code is unavailable.

## LICENSE / COPYING

Copyright 2016 Chaoya Li, Xiaoguang Zhu

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

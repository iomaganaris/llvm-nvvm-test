# llvm-nvvm-test
Testing LLVM IR compilation for GPU execution

Starting from an LLVM IR file with CUDA annotations generate executable code running on a GPU

nvvm module compiles LLVM IR code linking with `libdevice` to PTX code which then gets executed on a GPU

## Contents of repository

In `test_full.cpp` you can find the whole pipeline starting from LLVM IR code (`t2.ll`) up to transferring the needed data to GPU and executing the kernel.

`t2.ll` executes the `pow(a,b)` where `a` and `b` are elements of vectors `A` and `B` correspondingly and the result is stored in vector `C`.
The use of `pow` function introduces the need for linking of the executable with the `libdevice` bitcode which provides the implementation of the function.

In `kernel.ll` there is another kernel that does `C = A + B`.
The other `.cpp` files do either the translation from LLVM ir to PTX or the execution of the PTX file separately.


## Compilation

To compile the tests you need `LLVM` and `CUDA`. `LLVM` is needed to compile the C++ files and CUDA for the GPU execution and to provide NVVM and the `libdevice` math library.

You can find instructions in `compile.sh`.

`LLVM` and `CUDA` in this case are loaded using `environment modules`.

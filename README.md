# llvm-nvvm-test
Testing LLVM IR compilation for GPU execution

Starting from an LLVM IR file with CUDA annotations generate executable code running on a GPU

nvvm module compiles LLVM IR code linking with `libdevice` to PTX code which then gets executed on a GPU

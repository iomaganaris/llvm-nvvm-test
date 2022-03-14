#!/usr/bin/env bash

module load nvhpc llvm/13.0.0

# Simple test for addition
compile_sample() {
    llc -mcpu=sm_80 kernel.ll -o kernel.ptx
    clang++ sample.cpp -o sample -O2 -g -I${NVHPC_ROOT}/Linux_x86_64/22.1/cuda/11.5/include -lcuda
}

# libdevice test
compile_t2() {
    llvm-as t2.ll -o t2.bc
    ${LLVM_ROOT}/bin/llvm-link t2.bc ${NVHPC_ROOT}/Linux_x86_64/22.1/cuda/11.5/nvvm/libdevice/libdevice.10.bc -o t2.linked.bc
    opt -internalize -internalize-public-api-list=kernel --nvvm-reflect -O3 t2.linked.bc -o t2.opt.bc
    llc -mcpu=sm_60 t2.opt.bc -o t2.ptx
    clang++ t2.cpp -o t2 -O2 -g -I${NVHPC_ROOT}/Linux_x86_64/22.1/cuda/11.5/include -lcuda
}

compile_t2_link() {
    llc -mcpu=sm_60 t2.ll -o t2.ptx
    clang++ t2_link.cpp -o t2_link -O2 -g -I${NVHPC_ROOT}/Linux_x86_64/22.1/cuda/11.5/include -lcuda
}

compile_test() {
    clang++ test.cpp -o test_link -O2 -g -I${NVHPC_ROOT}/Linux_x86_64/22.1/cuda/11.5/include -I${NVHPC_ROOT}/Linux_x86_64/22.1/cuda/11.5/nvvm/include -lcuda -L${NVHPC_ROOT}/Linux_x86_64/22.1/cuda/nvvm/lib64 -lnvvm
    LD_LIBRARY_PATH=${NVHPC_ROOT}/Linux_x86_64/22.1/cuda/nvvm/lib64 ./test_link
}

compile_test_full() {
    clang++ test_full.cpp -o test_full -O2 -g -I${NVHPC_ROOT}/Linux_x86_64/22.1/cuda/include -I${NVHPC_ROOT}/Linux_x86_64/22.1/cuda/nvvm/include -lcuda -L${NVHPC_ROOT}/Linux_x86_64/22.1/cuda/nvvm/lib64 -lnvvm
    LD_LIBRARY_PATH=${NVHPC_ROOT}/Linux_x86_64/22.1/cuda/nvvm/lib64 ./test_full
}

compile_sample
compile_t2
compile_t2_link
compile_test
compile_test_full
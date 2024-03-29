#!/usr/bin/env bash

module load cuda/11.4.2 llvm/13.0.0

# Simple test for addition
compile_sample() {
    llc -mcpu=sm_80 kernel.ll -o kernel.ptx
    clang++ sample.cpp -o sample -O2 -g -I${CUDA_ROOT}/include -lcuda
    ./sample
}

# libdevice test
compile_t2() {
    llvm-as t2.ll -o t2.bc
    ${LLVM_ROOT}/bin/llvm-link t2.bc ${CUDA_ROOT}/nvvm/libdevice/libdevice.10.bc -o t2.linked.bc
    opt -internalize -internalize-public-api-list=kernel --nvvm-reflect -O3 t2.linked.bc -o t2.opt.bc
    llc -mcpu=sm_80 t2.opt.bc -o t2.ptx
    clang++ t2.cpp -o t2 -O2 -g -I${CUDA_ROOT}/include -lcuda
    LD_LIBRARY_PATH=${CUDA_ROOT}/nvvm/lib64 ./t2
}

compile_t2_link() {
    llc -mcpu=sm_80 t2.ll -o t2_link.ptx
    clang++ t2_link.cpp -o t2_link -O2 -g -I${CUDA_ROOT}/include -lcuda
    LD_LIBRARY_PATH=${CUDA_ROOT}/nvvm/lib64 ./t2_link
}

compile_test_full() {
    clang++ test_full.cpp -o test_full -O2 -g -I${CUDA_ROOT}/include -I${CUDA_ROOT}/nvvm/include -lcuda -L${CUDA_ROOT}/nvvm/lib64 -lnvvm
    LD_LIBRARY_PATH=${CUDA_ROOT}/nvvm/lib64 ./test_full
}

compile_sample
compile_t2
compile_t2_link # CUDA_ERROR_INVALID_PTX
compile_test_full
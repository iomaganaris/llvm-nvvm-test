#!/usr/bin/env bash

# Simple test for addition
compile_sample() {
    llc -mcpu=sm_60 kernel.ll -o kernel.ptx
    clang++ sample.cpp -o sample -O2 -g -I/home/magkanar/software/linux-ubuntu20.04-skylake/gcc-9.3.0/nvhpc-22.1-wg2mad/Linux_x86_64/22.1/cuda/11.5/include -lcuda
}

# libdevice test
compile_t2() {
    llvm-as t2.ll -o t2.bc
    /home/magkanar/software/linux-ubuntu20.04-skylake/gcc-9.3.0/llvm-13.0.0-otw5zk/bin/llvm-link t2.bc /home/magkanar/software/linux-ubuntu20.04-skylake/gcc-9.3.0/nvhpc-22.1-wg2mad/Linux_x86_64/22.1/cuda/11.5/nvvm/libdevice/libdevice.10.bc -o t2.linked.bc
    opt -internalize -internalize-public-api-list=kernel --nvvm-reflect -O3 t2.linked.bc -o t2.opt.bc
    llc -mcpu=sm_60 t2.opt.bc -o t2.ptx
    clang++ t2.cpp -o t2 -O2 -g -I/home/magkanar/software/linux-ubuntu20.04-skylake/gcc-9.3.0/nvhpc-22.1-wg2mad/Linux_x86_64/22.1/cuda/11.5/include -lcuda
}

compile_t2_link() {
    llc -mcpu=sm_60 t2.ll -o t2.ptx
    clang++ t2_link.cpp -o t2_link -O2 -g -I/home/magkanar/software/linux-ubuntu20.04-skylake/gcc-9.3.0/nvhpc-22.1-wg2mad/Linux_x86_64/22.1/cuda/11.5/include -lcuda
}

compile_test() {
    clang++ test.cpp -o test_link -O2 -g -I/home/magkanar/software/linux-ubuntu20.04-skylake/gcc-9.3.0/nvhpc-22.1-wg2mad/Linux_x86_64/22.1/cuda/11.5/include -I/home/magkanar/software/linux-ubuntu20.04-skylake/gcc-9.3.0/nvhpc-22.1-wg2mad/Linux_x86_64/22.1/cuda/11.5/nvvm/include -lcuda -L/home/magkanar/software/linux-ubuntu20.04-skylake/gcc-9.3.0/nvhpc-22.1-wg2mad/Linux_x86_64/22.1/cuda/nvvm/lib64 -lnvvm
    ./test_link
}

# compile_sample
compile_t2
compile_t2_link
compile_test
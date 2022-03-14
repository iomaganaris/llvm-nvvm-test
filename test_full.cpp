#include <iostream>
#include <fstream>
#include <cassert>
#include <cstring>
#include "cuda.h"
#include "nvvm.h"


void checkCudaErrors(CUresult err) {
    if (err != CUDA_SUCCESS) {
        const char *ret = NULL;
        cuGetErrorName(err, &ret);
        std::cerr << "CUDA error: " << ret << "\n";
        exit(1);
    }
}

void checkNVVMErrors(nvvmResult err) {
    if (err != NVVM_SUCCESS) {
        std::cerr << "NVVM Error: " << nvvmGetErrorString(err) << "\n";
        exit(1);
    }
}

std::string loadFile(const std::string& filename) {
    std::ifstream t(filename);
    if (!t.is_open()) {
        std::cerr << filename << " not found\n";
        return "";
    }
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
    return str;
}

/// main - Program entry point
int main(int argc, char **argv) {
    CUdevice    device;
    CUmodule    cudaModule;
    CUcontext   context;
    CUfunction  function;
    CUlinkState linker;
    int         devCount;

    // CUDA initialization
    checkCudaErrors(cuInit(0));
    checkCudaErrors(cuDeviceGetCount(&devCount));
    checkCudaErrors(cuDeviceGet(&device, 0));

    char name[128];
    checkCudaErrors(cuDeviceGetName(name, 128, device));
    std::cout << "Using CUDA Device [0]: " << name << "\n";

    int devMajor, devMinor;
    checkCudaErrors(cuDeviceGetAttribute(&devMajor, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR, device));
    checkCudaErrors(cuDeviceGetAttribute(&devMinor, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR, device));
    std::cout << "Device Compute Capability: "
                << devMajor << "." << devMinor << "\n";
    if (devMajor < 2) {
        std::cerr << "ERROR: Device 0 is not SM 2.0 or greater\n";
        return 1;
    }

    auto str = loadFile("t2.ll");
    //print str
    // std::cout << str << "\n";

    nvvmProgram prog;

    auto libdevice_str = loadFile("/home/magkanar/spack_software/linux-ubuntu20.04-skylake/gcc-11.1.0/cuda-11.4.2-ygshzt/nvvm/libdevice/libdevice.10.bc");
    const char *libdeviceMod = libdevice_str.c_str();
    size_t libdeviceModSize = libdevice_str.size();
    std::cout << "libdeviceModSize = " << libdeviceModSize << std::endl;
    const char *myIr = str.c_str();
    size_t myIrSize = str.size();

    // Create NVVM program object
    checkNVVMErrors(nvvmCreateProgram(&prog));

    // Add libdevice module to program
    checkNVVMErrors(nvvmAddModuleToProgram(prog, libdeviceMod, libdeviceModSize, "libdevice"));

    // Add custom IR to program
    checkNVVMErrors(nvvmAddModuleToProgram(prog, myIr, myIrSize, "myIr"));

    // Declare compile options
    const char *options[] = { "-arch=compute_80" };

    // Compile the program
    checkNVVMErrors(nvvmCompileProgram(prog, 1, options));

    // Get compiled module
    char* compiled_module;
    size_t compiled_module_size;
    checkNVVMErrors(nvvmGetCompiledResultSize(prog, &compiled_module_size));
    std::cout << "Compiled module size: " << compiled_module_size << "\n";
    compiled_module = (char*)malloc(compiled_module_size);
    checkNVVMErrors(nvvmGetCompiledResult(prog, compiled_module));
    std::cout << "Compiled module\n";
    // print compiled_module to file
    std::ofstream outfile;
    outfile.open("t2_test.ptx");
    outfile << compiled_module;
    outfile.close();

    std::string compiled_ptx_module{compiled_module};

    // Create driver context
    checkCudaErrors(cuCtxCreate(&context, 0, device));
    std::cout << "Created context" << std::endl;

    // Create module for object
    checkCudaErrors(cuModuleLoadDataEx(&cudaModule, compiled_module, 0, 0, 0));
    std::cout << "Loaded my LLVM IR module" << std::endl;

    // Get kernel function
    checkCudaErrors(cuModuleGetFunction(&function, cudaModule, "kernel"));

    // Device data
    CUdeviceptr devBufferA;
    CUdeviceptr devBufferB;
    CUdeviceptr devBufferC;

    checkCudaErrors(cuMemAlloc(&devBufferA, sizeof(float)*16));
    checkCudaErrors(cuMemAlloc(&devBufferB, sizeof(float)*16));
    checkCudaErrors(cuMemAlloc(&devBufferC, sizeof(float)*16));

    float* hostA = new float[16];
    float* hostB = new float[16];
    float* hostC = new float[16];

    // Populate input
    for (unsigned i = 0; i != 16; ++i) {
        hostA[i] = (float)i;
        hostB[i] = (float)(2*i);
        hostC[i] = 0.0f;
    }

    checkCudaErrors(cuMemcpyHtoD(devBufferA, &hostA[0], sizeof(float)*16));
    checkCudaErrors(cuMemcpyHtoD(devBufferB, &hostB[0], sizeof(float)*16));


    unsigned blockSizeX = 16;
    unsigned blockSizeY = 1;
    unsigned blockSizeZ = 1;
    unsigned gridSizeX  = 1;
    unsigned gridSizeY  = 1;
    unsigned gridSizeZ  = 1;

    // Kernel parameters
    void *KernelParams[] = { &devBufferA, &devBufferB, &devBufferC };

    std::cout << "Launching kernel\n";

    // Kernel launch
    checkCudaErrors(cuLaunchKernel(function, gridSizeX, gridSizeY, gridSizeZ,
                                    blockSizeX, blockSizeY, blockSizeZ,
                                    0, NULL, KernelParams, NULL));

    // Retrieve device data
    checkCudaErrors(cuMemcpyDtoH(&hostC[0], devBufferC, sizeof(float)*16));


    std::cout << "Results:\n";
    for (unsigned i = 0; i != 16; ++i) {
        std::cout << "pow(" << hostA[i] << ", " << hostB[i] << ") = " << hostC[i] << "\n";
    }


    // Clean up after ourselves
    delete [] hostA;
    delete [] hostB;
    delete [] hostC;

    // Clean-up
    checkCudaErrors(cuMemFree(devBufferA));
    checkCudaErrors(cuMemFree(devBufferB));
    checkCudaErrors(cuMemFree(devBufferC));
    checkCudaErrors(cuModuleUnload(cudaModule));
    checkCudaErrors(cuCtxDestroy(context));

    return 0;
}

#include <iostream>
#include <fstream>
#include <cassert>
#include "cuda.h"
#include "nvvm.h"


void checkCudaErrors(CUresult err) {
  assert(err == CUDA_SUCCESS);
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

    auto libdevice_str = loadFile("/home/magkanar/spack_software/linux-ubuntu20.04-skylake/gcc-11.1.0/nvhpc-22.1-zsmejw/Linux_x86_64/22.1/cuda/11.5/nvvm/libdevice/libdevice.10.bc");
    const char *libdeviceMod = libdevice_str.c_str();
    size_t libdeviceModSize = libdevice_str.size();
    std::cout << "libdeviceModSize = " << libdeviceModSize << std::endl;
    const char *myIr = str.c_str();
    size_t myIrSize = str.size();

    // Create NVVM program object
    nvvmCreateProgram(&prog);

    // Add libdevice module to program
    nvvmAddModuleToProgram(prog, libdeviceMod, libdeviceModSize, "libdevice");

    // Add custom IR to program
    nvvmAddModuleToProgram(prog, myIr, myIrSize, "myIr");

    // Declare compile options
    const char *options[] = { "-ftz=1" };

    // Compile the program
    nvvmCompileProgram(prog, 1, options);

    // Get compiled module
    char* compiled_module;
    size_t compiled_module_size;
    nvvmGetCompiledResultSize(prog, &compiled_module_size);
    std::cout << "Compiled module size: " << compiled_module_size << "\n";
    compiled_module = (char*)malloc(compiled_module_size);
    nvvmGetCompiledResult(prog, compiled_module);
    std::cout << "Compiled module\n";
    // print compiled_module to file
    std::ofstream outfile;
    outfile.open("t2_test.ptx");
    outfile << compiled_module;
    outfile.close();
}

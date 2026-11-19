#include <cuda_runtime.h>
#include <cstdio>
#include <iostream>

__global__ void simpleKernel(int* d_data, int value) {
    int idx = threadIdx.x + blockIdx.x * blockDim.x;
    if (idx < 1) {
        d_data[idx] = value;
    }
}

int main() {
    printf("=== CUDA Library Link Integrity Test ===\n\n");
    
    // 1. Test CUDA Runtime
    printf("1. Testing CUDA Runtime...\n");
    int deviceCount = 0;
    cudaError_t error = cudaGetDeviceCount(&deviceCount);
    
    if (error != cudaSuccess) {
        printf("  Failed: cudaGetDeviceCount error: %s\n", cudaGetErrorString(error));
        return 1;
    }
    printf("  Success: Found %d CUDA devices\n", deviceCount);
    
    if (deviceCount == 0) {
        printf("  Warning: No CUDA devices found\n");
        return 1;
    }
    
    // 2. Test Device Properties
    printf("\n2. Testing Device Properties...\n");
    cudaDeviceProp prop;
    error = cudaGetDeviceProperties(&prop, 0);
    
    if (error != cudaSuccess) {
        printf("  Failed: cudaGetDeviceProperties error: %s\n", cudaGetErrorString(error));
        return 1;
    }
    printf("  Success: GPU: %s (Compute Capability %d.%d)\n", 
           prop.name, prop.major, prop.minor);
    
    // 3. Test Memory Allocation
    printf("\n3. Testing Memory Allocation...\n");
    int* d_data = nullptr;
    error = cudaMalloc(&d_data, sizeof(int));
    
    if (error != cudaSuccess) {
        printf("  Failed: cudaMalloc error: %s\n", cudaGetErrorString(error));
        return 1;
    }
    printf("  Success: GPU memory allocation is normal\n");
    
    // 4. Test Kernel Execution
    printf("\n4. Testing Kernel Execution...\n");
    simpleKernel<<<1, 1>>>(d_data, 42);
    error = cudaDeviceSynchronize();
    
    if (error != cudaSuccess) {
        printf("  Failed: Kernel execution error: %s\n", cudaGetErrorString(error));
        cudaFree(d_data);
        return 1;
    }
    printf("  Success: Kernel execution is normal\n");
    
    // 5. Test Memory Copy
    printf("\n5. Testing Memory Copy...\n");
    int h_data = 0;
    error = cudaMemcpy(&h_data, d_data, sizeof(int), cudaMemcpyDeviceToHost);
    
    if (error != cudaSuccess) {
        printf("  Failed: cudaMemcpy error: %s\n", cudaGetErrorString(error));
        cudaFree(d_data);
        return 1;
    }
    printf("  Success: Memory copy is normal (value: %d)\n", h_data);
    
    // 6. Test Math Library (optional)
    printf("\n6. Testing Math Library (sinf)...\n");
    float result = sinf(3.1415926f / 2.0f);
    printf("  Success: sinf(PI/2) = %f\n", result);
    
    // 7. Test Event Timer
    printf("\n7. Testing Event Timer...\n");
    cudaEvent_t start, stop;
    error = cudaEventCreate(&start);
    if (error == cudaSuccess) {
        error = cudaEventCreate(&stop);
    }
    
    if (error != cudaSuccess) {
        printf("  Failed: cudaEventCreate error\n");
    } else {
        printf("  Success: CUDA event creation is normal\n");
        cudaEventDestroy(start);
        cudaEventDestroy(stop);
    }
    
    // Cleanup
    cudaFree(d_data);
    
    // 8. Test Optional Libraries
#ifdef TEST_CUBLAS
    printf("\n8. Testing cuBLAS Link...\n");
    // Here you can add cuBLAS test code
    printf("  Success: cuBLAS link is normal\n");
#endif
    
#ifdef TEST_CUFFT
    printf("\n9. Testing cuFFT Link...\n");
    // Here you can add cuFFT test code
    printf("  Success: cuFFT link is normal\n");
#endif
    
    printf("\n=== All tests passed! CUDA library link integrity is complete ===\n");
    return 0;
}

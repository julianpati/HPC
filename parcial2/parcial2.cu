#include <stdio.h>
#include <stdlib.h>
#include "someDefinitions.h"
#include <cuda.h>
#include <stdlib.h>

// Error handling macro
#define CUDA_CHECK(call) \
    if((call) != cudaSuccess) { \
        cudaError_t err = cudaGetLastError(); \
        printf("CUDA error calling, code is %d\n", err);}



__global__ void matrixMulKernel(double *d_a, double *d_b, double *d_c, int height, int width_a, int width_b) {
  int row = blockIdx.y * blockDim.y + threadIdx.y;
  int col = blockIdx.x * blockDim.x + threadIdx.x;

  if (row < height && col < width_b) {
    double p_result = 0;
    for (int k = 0; k < width_a; k++) {
      p_result += d_a[row * width_a + k] * d_b[k * width_b + col];
    }
    d_c[row * width_b + col] = p_result;
  }
}

void cudaCall(int f1, int c1, int f2, double* M1, double* M2, double* M3){
	double *d_M, *d_N, *d_P;
	cudaMalloc(&d_M, sizeof(double) * f1 * c1);
	cudaMalloc(&d_N, sizeof(double) * c1 * f2);
	cudaMalloc(&d_P, sizeof(double) * f1 * f2);
  
	cudaError_t error = cudaSuccess;

	error = cudaMemcpy(d_M, M1, f1 * c1 * sizeof(double), cudaMemcpyHostToDevice);
	if(error != cudaSuccess){
		printf("Error copiando datos a d_M");
		exit(0);
	}

	error = cudaMemcpy(d_N, M2, c1 * f2 * sizeof(double), cudaMemcpyHostToDevice);
	if(error != cudaSuccess){
		printf("Error copiando datos a d_N");
		exit(0);
	}

	int blockSize = 32;
	dim3 dimBlock(blockSize,blockSize,1);
	dim3 dimGrid(ceil(f1/double(blockSize)),ceil(f2/double(blockSize)),1);
	matrixMulKernel<<<dimGrid,dimBlock>>>(d_M,d_N,d_P, f1, c1, f2);
	cudaDeviceSynchronize();
	cudaMemcpy(M3,d_P, f1 * f2 * sizeof(double),cudaMemcpyDeviceToHost);
	cudaFree(d_M);
    cudaFree(d_N);
    cudaFree(d_P);
	//Fin multiplicacion con GPU
}

#ifndef  __MEDIANFILTER_CU_
#define  __MEDIANFILTER_CU_

#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <time.h>
#include <iostream>
#define datasize 100

inline void checkCudaErrors(cudaError err) //cuda error handle function
{
	if (cudaSuccess != err)
	{
		fprintf(stderr, "CUDA Runtime API error:%s.\n", cudaGetErrorString(err));
		return;
	}
}

__device__ int rs_normaliseXY(int x, int y, int max_x, int max_y) {
	if (x >= max_x || x <= 0 || y >= max_y || y <= 0) //�����߽�Ĳ����ú�ɫ���
		return 0;
	return x;
}
 __device__ int rs_checkColorSpace(double x) {
	if (x > 255)
		return 255;
	if (x < 0)
		return 0;
	return x;
}
__device__ float rs_BiCubicPoly(float x)
{
	float abs_x = abs(x);
	float a = -0.5;
	if (abs_x <= 1.0)
	{
		return (a + 2)*pow(abs_x, 3) - (a + 3)*pow(abs_x, 2) + 1;
	}
	else if (abs_x < 2.0)
	{
		return a*pow(abs_x, 3) - 5 * a*pow(abs_x, 2) + 8 * a*abs_x - 4 * a;
	}
	else
		return 0.0;
}
__global__ void Rotate_And_Scale(int *In, int *Out, int Width, int Height,double angle,double factor)
{
	double M_PI = 3.1415926;
	//  Degrees-to-radians constant 
	const double  DegreesToRadians = M_PI / 180.0;
	//�������
	double theta = angle * DegreesToRadians;// ��ת�ĽǶ�
	int y = blockDim.y * blockIdx.y + threadIdx.y;
	int x = blockDim.x * blockIdx.x + threadIdx.x;
	if (x <= Width && x >= 0 && y <= Height && y >= 0)
	{

		//������ת���v,w���п��ܴ���С��
		double real_v = x*cos(theta) + y*sin(theta) + cos(theta)*(-1)*(Width / 2) + sin(theta)*(-1)*(Height / 2) ;
		double real_w = (-1)*x*sin(theta) + y*cos(theta) + cos(theta)*(-1)*(Height / 2) - sin(theta)*(-1)*(Width / 2) ;
		//�����ź�ƽ��
		real_v = real_v / factor - (-1)*Width / 2;
		real_w = real_w / factor - (-1)*Height / 2;
		//��ÿһ��λ�ã�x,y��ʹ��(v,w) = T^(-1)(x,y)��������ͼ���е���Ӧλ�� 
		double value = 0;
		int v, w;
		//ȡ��������
		v = floor(real_v);
		w = floor(real_w);
		if ((value = rs_normaliseXY(v, w, Width, Height)) != 0)//��û�����߽�Ļ�
			if (v >= 2 && v < Width - 2 && w >= 2 && w < Height - 2) {//��ֹԽ��														
				//ʵ��˫�����ڲ�
				value = 0;
				//4*4��������
				for (int i = -1; i < 3; i++)
					for (int j = -1; j < 3; j++)
						value += (*(In + Width * (w + i) + (v + j)))*rs_BiCubicPoly(real_v - v - j)*rs_BiCubicPoly(real_w - w - i);
			}
		value = rs_checkColorSpace(value);
		Out[y* Width + x] = value;
	}
}

extern "C" void rotate_and_scale_host(int *pixel, int Width, int Height,double angle, double factor)
{
	int *pixelIn, *pixelOut;
	dim3 dimBlock(32, 32);
	dim3 dimGrid((Width + dimBlock.x - 1) / dimBlock.x, (Height + dimBlock.y -
		1) / dimBlock.y);
	checkCudaErrors(cudaMalloc((void**)&pixelIn, sizeof(int) * Width * Height));
	checkCudaErrors(cudaMalloc((void**)&pixelOut, sizeof(int) * Width * Height));

	checkCudaErrors(cudaMemcpy(pixelIn, pixel, sizeof(int) * Width * Height, cudaMemcpyHostToDevice));

	Rotate_And_Scale << <dimGrid, dimBlock >> > (pixelIn, pixelOut, Width , Height, angle,factor);

	checkCudaErrors(cudaMemcpy(pixel, pixelOut, sizeof(int) * Width * Height, cudaMemcpyDeviceToHost));


	cudaFree(pixelIn);
	cudaFree(pixelOut);
}

#endif // ! __MEDIANFILTER_KERNEL_CU_
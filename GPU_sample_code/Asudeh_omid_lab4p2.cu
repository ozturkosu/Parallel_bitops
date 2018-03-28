#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "read_bmp.h"
#include "read_bmp_clib.h"
// the kernel
__global__ 
void sobel(uint8_t *bmp_data,uint8_t *new_bmp_img,int threshold, int ht, int wd) 
{
	int block_origin_x = blockDim.x*blockIdx.x;
	int block_origin_y = blockDim.y*blockIdx.y;
	int pixelX = block_origin_y+threadIdx.x;
	int pixelY = block_origin_x+threadIdx.y;
	if(pixelX>0 && pixelX<wd-1 && pixelY>0 && pixelY<ht-1) // if not border
	{

		int Gx = bmp_data[(pixelY-1)*wd +(pixelX+1)] - bmp_data[(pixelY-1)*wd+(pixelX-1)] +
		       2*bmp_data[(pixelY)  *wd +(pixelX+1)] - 2*bmp_data[(pixelY)  *wd +(pixelX-1)] +
			 bmp_data[(pixelY+1)*wd +(pixelX+1)] - bmp_data[(pixelY+1)*wd+(pixelX-1)];

		int Gy = +bmp_data[(pixelY-1)*wd+(pixelX-1)] + 2*bmp_data[(pixelY-1)*wd+(pixelX)] + bmp_data[(pixelY-1)*wd+(pixelX+1)]
		  -bmp_data[(pixelY+1)*wd+(pixelX-1)] - 2*bmp_data[(pixelY+1)*wd+(pixelX)] - bmp_data[(pixelY+1)*wd +(pixelX+1)];	 

		float mag = sqrtf(Gx*Gx+Gy*Gy);
		if(mag>threshold)
			new_bmp_img[(pixelY)*wd+(pixelX)] = 255;
		else
			new_bmp_img[(pixelY)*wd+(pixelX)] = 0;
	}

}
int main(int argc,char** argv)
{
	char* input_img_file = argv[1];
	char* serial_img = argv[2];
	char* cuda_img = argv[3];
	//============== reading the binary bmp file into buffer ==============
	FILE *input_file;
	input_file = fopen(input_img_file,"rb");
	//=====================================================================	
	printf("**********************************************************************\n");
	printf("please wait. The serial version may take up to a minute to run\n");
	//1. Host memory allocation and getting the image and its attributes	
	bmp_image img;
	uint8_t *bmp_data;
	bmp_data = (uint8_t *)img.read_bmp_file(input_file);
	// allocate new output buffer of the same size for the sobel image
	uint8_t *new_bmp_img;
	new_bmp_img = (uint8_t *) malloc(img.num_pixel);
	//get image attributes
	int wd = img.image_width;	int ht = img.image_height;
//############################################## Serial Code #################################################################
	//############# convergence loop #############################
	int threshold = 0;
	int black_cell_count = 0;
	clock_t t;
	t = clock();
	while(black_cell_count<(75*wd*ht/100))
	{
		black_cell_count = 0;
		threshold+=1;
		for(int i = 1;i<(ht-1);i++)
		{
			for(int j=1;j<(wd-1);j++)
			{
				int Gx = bmp_data[(i-1)*wd +(j+1)] - bmp_data[(i-1)*wd +(j-1)] +
					2*bmp_data[(i)*wd +(j+1)] - 2*bmp_data[(i)*wd +(j-1)] +
					bmp_data[(i+1)*wd +(j+1)] - bmp_data[(i+1)*wd +(j-1)];

				int Gy = bmp_data[(i-1)*wd +(j-1)] + 2*bmp_data[(i-1)*wd +(j)] + bmp_data[(i-1)*wd +(j+1)]
					-bmp_data[(i+1)*wd +(j-1)] - 2*bmp_data[(i+1)*wd +(j)] - bmp_data[(i+1)*wd +(j+1)];	 
				float mag = sqrt(Gx*Gx+Gy*Gy);
				if(mag>threshold)
					new_bmp_img[i*wd+j] = 255;
				else
				{
					new_bmp_img[i*wd+j] = 0;
					black_cell_count++;
				}				
			}
		}
	}
	t = clock()-t;
	printf("Time taken for serial sobel operation:%f sec\n",((float)t)/CLOCKS_PER_SEC);
	printf("Threshold during convergence:%d\n\n",threshold);

	//write back the new bmp image into serial output file
	FILE *output_file;
	output_file = fopen(serial_img,"wb");
	img.write_bmp_file(output_file, new_bmp_img);
	bmp_data = (uint8_t *)img.read_bmp_file(input_file);
//############################################## CUDA Code ###################################################################
	// allocate new output buffer of the same size for the sobel image
	uint8_t *new_bmp_img1;
	new_bmp_img1 = (uint8_t *) malloc(img.num_pixel);
	//2. define thread hirearchy
	// the Grid has 32*32 blocks
	int GridDimX = ceil((float)ht/32);// Grid X dimension 
	int GridDimY = ceil((float)wd/32);// Grid Y dimension
	// each block has 32*32 threads;the load balancing is one pixel per thread	
	int BlockDimX = 32;  
	int BlockDimY = 32;
	
	//3. Device Memory allocation 
	uint8_t *bmp_data_device;//pointer to device version of the bmp_data,
	uint8_t *new_bmp_img_device;//pointer to device version of the output sobel image new_bmp_img
	cudaMalloc( (void**)&bmp_data_device, img.num_pixel);
	cudaMalloc( (void**)&new_bmp_img_device, img.num_pixel);
	
	//4. copy bmp_data,black_cell_count_array_host to the device
	cudaMemcpy( bmp_data_device, bmp_data, img.num_pixel, cudaMemcpyHostToDevice ); 

	//5. lunch kernel
	dim3 dimBlock(BlockDimX,BlockDimY);// 
	dim3 dimGrid(GridDimX,GridDimY);//  
	//############# convergence loop #############################
	threshold = -1;
	black_cell_count = 0;
	t = clock();
	while(black_cell_count<(75*(img.num_pixel)/100))
	{
		black_cell_count = 0;
		threshold+=1;
		// let the GPU calculate the sobel image
		sobel<<<dimGrid, dimBlock>>>(bmp_data_device,new_bmp_img_device,threshold,ht,wd);
		//copy back the black_cell_count_array from the GPU
		cudaMemcpy(new_bmp_img1 ,new_bmp_img_device, img.num_pixel, cudaMemcpyDeviceToHost );		
		for(int i = 1; i< (ht-1);i++)
		{
			for(int j=1;j<wd-1;j++)
				if(new_bmp_img1[i*wd+j]==0)
					black_cell_count++;
		}
		
	}
	t = clock()-t;
	printf("Time taken for CUDA sobel operation:%f sec\n",((float)t)/CLOCKS_PER_SEC);
	printf("Threshold during convergence:%d\n",threshold);
	printf("**********************************************************************\n");

	//7. free GPU memory
	cudaFree( bmp_data_device );
	cudaFree( new_bmp_img_device);

	//8. write back the new bmp image into output file
	FILE *output_file1;
	output_file1 = fopen(cuda_img,"wb");
	img.write_bmp_file(output_file1, new_bmp_img1);
	
	return 0;
}

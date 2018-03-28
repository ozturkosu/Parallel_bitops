#include <stdio.h>
#include <time.h>
void print_matrix(double * A, int dim)
{
	for(int i=0;i<dim;i++)
	{
		for(int j=0;j<dim;j++)
			printf("%f\t",A[i*dim+j]);
		printf("\n");
	}
}
// the kernel
__global__ 
void mult_transpose(double *A,double *C, int dim) 
{
	// Each thread with the id = tid, will calculate the tid-th row of the result matrix
	int tid = blockIdx.x*blockDim.x+threadIdx.x; // which thread you are? what is your tid?
	// go calculate your row! (tid-th row)
	for(int j = 0;j<dim;j++)
	{
		double sum = 0;
		for(int k=0;k<dim;k++)
			sum+=(A[j*dim+k]*A[k*dim+j]);//sum+=A[j][k]*A[k][j];
		C[tid*dim+j]= sum;//C[tid][j] = sum;
	}
}
//test 
int main()
{
//############################################ Serial Code ############################################################
	printf("*****************************************************************************************\n");
	int dim = 1024;
	int num_elem = dim*dim;
	int array_size = num_elem*sizeof(double);	
	double *a = (double*) malloc(array_size);
	
	// initialize the array
	//here I just initialize the input array to all one
	for(int i = 0;i<num_elem;i++)
		a[i] = 1;
	//main loop for serial version
	double *result = (double*) malloc(array_size); // the resut array
	clock_t t;
	t = clock();
	for(int i = 0;i<dim;i++)
	{
		for(int j=0;j<dim;j++)
		{	
			double sum = 0;
			for(int k=0;k<dim;k++)
				sum+=(a[i*dim+k]*a[k*dim+j]);
			result[i*dim+j] = sum;
		}
	}
	t = clock()-t;
	printf("Time taken for serial operation:%f seconds\n",((float)t)/CLOCKS_PER_SEC);
//############################################# CUDA Code #############################################################
	// 1. define thread hirearchy
	int num_blocks = 4;
	int num_th_per_blk = 1024;// we are not violating the nvidea 1024 threads per block limitation 
	int dim1 = 4096;
	int num_elem1 = dim1*dim1;
	int array_size1 = num_elem1*sizeof(double);

	//2. Host memory allocation	
	double *a1 = (double*) malloc(array_size1);  //pointer to host version of the input array	
	double *result1 = (double*) malloc(array_size1); // the result array
	// initialize the array
	//here I just initialize the input array to all one
	for(int i = 0;i<num_elem1;i++)
		a1[i] = 1;
	//3. Device Memory allocation 
	double *ad, *C;			     //pointer to device version of the array, and the output array C
	cudaMalloc( (void**)&ad, array_size1);
	cudaMalloc( (void**)&C, array_size1);

	//4. copy array to the device
	cudaMemcpy( ad, a1, array_size1, cudaMemcpyHostToDevice ); 

	//5. lunch kernel
	dim3 dimBlock(num_th_per_blk);//  1D thread hirearchy. In this case 1024 thread per block
	dim3 dimGrid(num_blocks);//  In this 4 blocks in the grid
	t = clock();
	mult_transpose<<<dimGrid, dimBlock>>>(ad,C, dim1);
	
	//6. copy back the result
	
	cudaMemcpy( result1, C, array_size1, cudaMemcpyDeviceToHost ); 
	t = clock()-t;
	printf("Time taken for CUDA operation:%f seconds\n",((float)t)/CLOCKS_PER_SEC);

	//7. free GPU memory
	cudaFree( ad );
	cudaFree( C );
		
	printf("*****************************************************************************************\n");
	return 0;
}

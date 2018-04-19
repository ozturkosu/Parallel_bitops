#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>
#include <time.h>
#include "read_bmp.h"
# define NUM_OF_THREADS 1
int main(int argc, char**argv)
{

//############################################## MPI Code #################################################################

MPI_Init(&argc,&argv);
	char* input_img_file = argv[1];
	char* output_img = argv[2];
	//============== each process reads a copy of the input image ==============
	FILE *input_file;
	input_file = fopen(input_img_file,"rb");
	uint8_t *bmp_data;
	bmp_data = (uint8_t *)read_bmp_file(input_file);
	//get image attributes
	int wd = get_image_width();	
	int ht = get_image_height();
	int num_pixels = wd*ht;
	
	//barrier. wait till all processes read the image
	MPI_Barrier(MPI_COMM_WORLD);
	
	// get the rank and communicator size
	int rank, comm_size;
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&comm_size);

uint8_t *new_bmp_img= NULL;
if(rank == 0)// assign the serial version to the process number 0
{
	//allocate memory for the ouput image
	int mem_size = num_pixels*sizeof(uint8_t);
	//printf("output image size:%d\n",mem_size);
	new_bmp_img = malloc(mem_size);
	// initiate the output image to all zero
	int i;
	for(i=0;i<mem_size;i++)
		new_bmp_img[i]=0;
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
		int i;
		for( i = 1;i<(ht-1);i++)
		{
			int j;
			for( j=1;j<(wd-1);j++)
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
	printf("********************************************************\n");
	printf("Time taken for serial sobel operation:%f sec\n",((float)t)/CLOCKS_PER_SEC);
	printf("Threshold during convergence:%d\n\n",threshold);	
}
	//############################################## MPI Code #################################################################
	// If master, initiate threshold and black_cell_count	
	int threshold,black_cell_count;
	if(rank == 0) // If Master
	{
		threshold = 0;
		black_cell_count = 0;
	}
	MPI_Barrier(MPI_COMM_WORLD);// makes sure that threshold and black_cell_count are initiated by process 0 
	MPI_Bcast(&threshold, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&black_cell_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);// make sure that all got threshold and black_cell_count
	//================================================
	
	// If Master, set the start time stamp
	clock_t t;
	if(rank == 0) 
		t = clock();//start timer

	// initiate the local_black_cell_count 
	int local_black_cell_count = 0;	

	//calculate the chunk height 
	int chunk_ht = (ht/comm_size);
	if(rank == 0)// last chunk
		chunk_ht = ht - (comm_size-1)*chunk_ht;
	// allocate memory for the local ouput
	int chunk_size =(chunk_ht)*wd;
	uint8_t *local_new_bmp_img;
	int mem_size_local = chunk_size*sizeof(uint8_t);
	local_new_bmp_img =  malloc(mem_size_local);	
	//printf("%d:local chunck size:%d\n",rank,mem_size_local);
	//calculate the working area of the current process
	int first_row,last_row;
	first_row = (rank-1)*chunk_ht+1;
	last_row = (rank)*chunk_ht;
	//printf("chunk_ht:%d\n",chunk_ht);
	if(rank == 0)// if the last chunk
	{	
		last_row = ht-2;//extend the chunk to the end and exclude the last row of the image	
		first_row = ht - chunk_ht+1;
	}		
	//############# convergence loop #############################
	while(black_cell_count<(75*wd*ht/100))
	{
		local_black_cell_count = 0;
		threshold++;
		
		omp_set_num_threads(NUM_OF_THREADS);
		#pragma omp parallel
		{
			int num_threads = omp_get_num_threads();
			int tid = omp_get_thread_num();
			int thread_ht = (last_row-first_row+1)/num_threads;	
			int i =0;
			//================================ load sharing between threads
			int low,high;
			if(tid!=num_threads)// if not last thread
			{
				low = first_row+tid*thread_ht;
				high= first_row+(tid+1)*thread_ht-1; 
			}
			else// if last thread
			{
				low = low = first_row+tid*thread_ht;;
				high= last_row;
			}
			//================================
			#pragma omp parallel for reduction (+:local_black_cell_count)			
			for(i = low;i<= high;i++) 
			{
				int j;
				for(j=1;j<(wd-1);j++)// exclude the first and the last columns
				{
					int Gx = bmp_data[(i-1)*wd +(j+1)] - bmp_data[(i-1)*wd +(j-1)] +
						2*bmp_data[(i)*wd +(j+1)] - 2*bmp_data[(i)*wd +(j-1)] +
						bmp_data[(i+1)*wd +(j+1)] - bmp_data[(i+1)*wd +(j-1)];
						int Gy = bmp_data[(i-1)*wd +(j-1)] + 2*bmp_data[(i-1)*wd +(j)] + bmp_data[(i-1)*wd +(j+1)]
						-bmp_data[(i+1)*wd +(j-1)] - 2*bmp_data[(i+1)*wd +(j)] - bmp_data[(i+1)*wd +(j+1)];	 
					float mag = sqrt(Gx*Gx+Gy*Gy);
					if(mag>threshold)
						local_new_bmp_img[(i%chunk_ht)*wd+j] = 255;
					else
					{
						local_new_bmp_img[(i%chunk_ht)*wd+j] = 0;
						local_black_cell_count++;	
					}	
				}
			}
		}
		//Reduce
		MPI_Allreduce(&local_black_cell_count, &black_cell_count, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
		//printf("%d,%d\n",rank,chunk_size);
	}

	if(rank!=0)
	{
		MPI_Send(
		local_new_bmp_img,
		chunk_size,
		MPI_UNSIGNED_CHAR,
		 0,
		 0,
		MPI_COMM_WORLD);
		//printf("%d : %d sent\n",rank,chunk_size);
	}
	//printf("%d,%d\n",rank,chunk_size);
	if(rank == 0) // If Master
	{
		int i;
		for(i = 1;i<comm_size;i++)
		{
			int t = (ht/comm_size);
			int ch_size = t*wd;
			
			MPI_Recv(
			&(new_bmp_img[(i-1)*ch_size]),
			ch_size,
			MPI_UNSIGNED_CHAR,
			i,
			0,
			MPI_COMM_WORLD,
			MPI_STATUS_IGNORE);
			//printf("%d : %d, received at %d\n",i,ch_size,(i-1)*ch_size);

		}
		//copy process 0 local_new_bmp_img to the new_bmp_img
		int j;
		//printf("%d\n",chunk_size);
		for( i = num_pixels;i>num_pixels-chunk_size;i--)
			new_bmp_img[i] = local_new_bmp_img[i%chunk_size];
		t = clock()-t;		//stop  timer
		printf("Time taken for MPI sobel operation:%f sec\n",((float)t)/CLOCKS_PER_SEC);
		printf("Threshold during convergence:%d\n\n",threshold);
		printf("********************************************************\n");
		//write back the new bmp image into output file
		FILE *output_file;
		output_file = fopen(output_img,"wb");
		write_bmp_file(output_file, new_bmp_img);
	}
	
MPI_Finalize();
//#########################################################################################################################
		
	return 0;
}

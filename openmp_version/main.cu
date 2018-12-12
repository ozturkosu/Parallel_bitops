#include<iostream>
#include "mybitops.h"
#include <fstream>
#include <cstdlib>
#include <cuda.h>
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>


//Project Fpr GPU classes

using namespace std;
#define DATA_GENERATION true

#define CHUNK_SIZE 3200


__device__ int word_type_device(size_t word1){

	if((word1 & 0x80000000) == 0)
		return 0; // literal word
	else{// fill word
		if((word1 & 0xc0000000) == 0xc0000000)
		return 2;//ones
		else
		return 1;//zeros

	}

}

__device__ int ismyfill_device(size_t word1){

	if((word1 & 0x80000000) == 0)
		return 0; // literal word
	else
		return 1; // fill word

}

__device__ size_t myBinarySearchGPU(size_t* arr, size_t l, size_t r, size_t x)
{
	if(x==0)
		return 0;
	if (r >= l)
	{
		size_t mid = l + (r - l)/2;
		// If the element is present at the middle
		// itself
		if (arr[mid] == x)
			return mid;
		// If element is smaller than mid, then
		// it can only be present in left subarray
		if (arr[mid] > x)
			return myBinarySearchGPU(arr, l, mid-1, x);
		// Else the element can only be present
		// in right subarray
		return myBinarySearchGPU(arr, mid+1, r, x);
	}

	// We reach here when element is not
	// present in array
	if(r<0)
		return 0;
	return r;

}

/*

__global__ void parallelAndDevice2(size_t * Vector1, int Vector1_size,  size_t * Vector2,
	int Vector2_size ,size_t * outVector, size_t vector1_bit_length, size_t vector2_bit_length)
{
		 int tid= blockIdx.x * blockDim.x + threadIdx.x ;

		 size_t min_bit_length = (vector1_bit_length<vector2_bit_length)?vector1_bit_length:vector2_bit_length;

		 size_t* word_lengths1 = new size_t[vec1_size]; //vector of length of the words
 		 size_t* prefix_sum1 = new size_t[vec1_size]; // prefix-sum generated form word_lengths1
 		 size_t pre_sum1 = 0;
 		 size_t vector1_bit_length = 0;

		 //Generate prefix sum and words size array for vector 1
		 for(int i = 0 ; i<vec1_size;i++)
		 {
			 size_t word = comp_bitset1[i];
			 int wt = Bitops.word_type(word);
			 size_t word_length = 31;
			 if(wt != 0)// if a fill word
				 word_length = word & 0x3fffffff;
			 word_lengths1[i] = word_length;
			 prefix_sum1[i] = pre_sum1+word_length;
			 pre_sum1 = prefix_sum1[i];
			 vector1_bit_length+=word_length;
		 }



		 size_t* word_lengths2 = new size_t[vec2_size]; //vector of length of the words
		 size_t* prefix_sum2 = new size_t[vec2_size];   // prefix-sum generated form word_lengths1
		 size_t pre_sum2 = 0;
		 size_t vector2_bit_length = 0;

		 for(int i = 0 ; i<vec2_size;i++)
		 {
			 size_t word = comp_bitset2[i];
			 int wt = Bitops.word_type(word);
			 size_t word_length = 31;
			 if(wt != 0)// if a fill word
				 word_length = word & 0x3fffffff;
			 word_lengths2[i] = word_length;
			 prefix_sum2[i] = pre_sum2+word_length;
			 // printf("%u\n",word_length);
			 pre_sum2 = prefix_sum2[i];
			 vector2_bit_length+=word_length;
		 }




}

*/

__global__ void parallelAndDevice(size_t * Vector1, int Vector1_size, size_t * prefixSum1, size_t * wordSize1  , size_t * Vector2,
	int Vector2_size, size_t * prefixSum2 , size_t * wordSize2 ,size_t * outVector, size_t vector1_bit_length, size_t vector2_bit_length)
{
	 //comp_bitset1_device[0]=1;
	 int tid= blockIdx.x * blockDim.x + threadIdx.x ;

	 size_t min_bit_length = (vector1_bit_length<vector2_bit_length)?vector1_bit_length:vector2_bit_length;

	 size_t *v1_ptr = &Vector1[0];
	 size_t *v2_ptr = &Vector2[0];


	 size_t b1 = tid*CHUNK_SIZE;
	 size_t b2 = (tid+1)*CHUNK_SIZE;

	 ////2.2 calc the word area
	 int w11 = myBinarySearchGPU(prefixSum1,0,Vector1_size-1,b1);
	 int w12 = myBinarySearchGPU(prefixSum1,0,Vector1_size-1,b2);

	 int w21 = myBinarySearchGPU(prefixSum2,0,Vector2_size-1,b1);
	 int w22 = myBinarySearchGPU(prefixSum2,0,Vector2_size-1,b2);

	 float res_size = ((w12-w11)<(w22-w21))?(w12-w11):(w22-w21);

	 size_t min_length = res_size * 1.01;//give 1% more capacity
	 //size_t *result_vector = new size_t[min_length] ;
	 size_t *result_vector = &outVector[0] ;
	 size_t *result_ptr = &result_vector[0] ;

	 size_t ones = 0, zeros = 0;
	 size_t it1 = w11;
	 size_t it2 = w21;
	 size_t word1 = Vector1[it1];
	 size_t word2 = Vector2[it2];
	 size_t count = 0;


	 while(it1 <= w12 && it2 <= w22) {
		 if(ismyfill_device(word1) && ismyfill_device(word2)) {  //both are fill words
			 size_t c1 = word1 & 0x3fffffff;         //get the len of fill word
			 size_t c2 = word2 & 0x3fffffff;         //get the len of fill word
			 size_t isone1 = word1 & 0x40000000;     //check if it is one
			 size_t isone2 = word2 & 0x40000000;     //check if it is one

			 if(c1 == c2) {
				 if(isone1 && isone2) {
					 //allZeros = false;
					 ones += c1;
					 if(zeros > 0) {
						 size_t n = 0x80000000 + zeros;
						 zeros = 0;
						 result_ptr[count] = n;
						 count++;
						 // result_vector.push_back(n);
					 }
				 } else {
					 zeros += c1;
					 if(ones > 0) {
						 size_t n = 0xc0000000 + ones;
						 ones = 0;
						 result_ptr[count] = n;
						 count++;
						 // result_vector.push_back(n);
					 }
				 }
				 word1 = Vector1[++it1];
				 word2 = Vector2[++it2];
			 } else if (c1 > c2) {
				 if(isone1 && isone2) {
					 //allZeros = false;
					 ones += c2;
					 if(zeros > 0) {
						 size_t n = 0x80000000 + zeros;
						 zeros = 0;
						 result_ptr[count] = n;
						 count++;
						 // result_vector.push_back(n);
					 }
				 } else {
					 zeros += c2;
					 if(ones > 0) {
						 size_t n = 0xc0000000 + ones;
						 ones = 0;
						 // result_vector.push_back(n);
					 }
				 }
				 if(isone1)
					 word1 = 0xc0000000 + (c1 - c2);
				 else
					 word1 = 0x80000000 + (c1 - c2);
				 word2 = Vector2[++it2];
			 } else { //c2 > c1
				 if(isone1 && isone2) {
					 //allZeros = false;
					 ones += c1;
					 if(zeros > 0) {
						 size_t n = 0x80000000 + zeros;
						 zeros = 0;
						 result_ptr[count] = n;
						 count++;
						 // result_vector.push_back(n);
					 }
				 } else {
					 zeros += c1;
					 if(ones > 0) {
						 size_t n = 0xc0000000 + ones;
						 ones = 0;
						 result_ptr[count] = n;
						 count++;
						 // result_vector.push_back(n);
					 }
				 }
				 if(isone2)
					 word2 = 0xc0000000 + (c2 - c1);
				 else
					 word2 = 0x80000000 + (c2 - c1);
				 word1 = Vector1[++it1];
			 }
		 } else if(ismyfill_device(word1) && !ismyfill_device(word2)) { //one fill, one literal
			 size_t c1 = word1 & 0x3fffffff; //lenth
			 size_t isone1 = word1 & 0x40000000;  //is one?

			 if(isone1) { //if one, depend on literal
				 if(ones > 0) {
					 size_t n = 0xc0000000 + ones;
					 //allZeros = false;
					 ones = 0;
					 result_ptr[count] = n;
					 count++;
					 // result_vector.push_back(n);
				 }
				 if(zeros > 0) {
					 size_t n = 0x80000000 + zeros;
					 zeros = 0;
					 result_ptr[count] = n;
					 count++;
					 // result_vector.push_back(n);
				 }
				 result_ptr[count] = word2;
				 count++;
				 // result_vector.push_back(word2);
			 } else { //id zero, generate a zero word
				 if(ones > 0) {
					 size_t n = 0xc0000000 + ones;
					 ones = 0;
					 result_ptr[count] = n;
					 count++;
					 // result_vector.push_back(n);
				 }
				 zeros += 31;
			 }
			 if(c1 == 31){
				 word1 = Vector1[++it1];
			 }
			 else {
				 if(isone1)
					 word1 = 0xc0000000 + (c1 - (size_t)31);
				 else
					 word1 = 0x80000000 + (c1 - (size_t)31);
			 }
			 word2 = Vector2[++it2];
		 } else if(!ismyfill_device(word1) && ismyfill_device(word2)) { //one literal, one fill
			 size_t c2 = word2 & 0x3fffffff; //lenth
			 size_t isone2 = word2 & 0x40000000;  //is one?

			 if(isone2) { //if one, depend on literal
				 if(ones > 0) {
					 size_t n = 0xc0000000 + ones;
					 //allZeros = false;
					 ones = 0;
					 result_ptr[count] = n;
					 count++;
					 // result_vector.push_back(n);
				 }
				 if(zeros > 0) {
					 size_t n = 0x80000000 + zeros;
					 zeros = 0;
					 result_ptr[count] = n;
					 count++;
					 // result_vector.push_back(n);
				 }
				 //if (word1 != 0) //allZeros = false;
				 result_ptr[count] = word1;
				 count++;
				 // result_vector.push_back(word1);
			 } else { //id zero, generate a zero word
				 if(ones > 0) {
					 size_t n = 0xc0000000 + ones;
					 ones = 0;
					 result_ptr[count] = n;
					 count++;
					 // result_vector.push_back(n);
				 }
				 zeros += 31;
			 }
			 if(c2 == 31)
				 word2 = Vector2[++it2];
			 else {
				 if(isone2)
					 word2 = 0xc0000000 + (c2 - (size_t)31);
				 else
					 word2 = 0x80000000 + (c2 - (size_t)31);
			 }
			 word1 = Vector1[++it1];
		 } else { //both are literal words
			 size_t num = word1 & word2;
			 if(num == 0) {
				 zeros += 31;
				 if(ones > 0) {
			 //allZeros = false;
					 size_t n = 0xc0000000 + ones;
					 ones = 0;
					 result_ptr[count] = n;
					 count++;
					 // result_vector.push_back(n);
				 }
			 } else if (num == 0x7fffffff) {
				 ones += 31;
				 //allZeros = false;
				 if(zeros > 0) {
					 size_t n = 0x80000000 + zeros;
					 zeros = 0;
					 result_ptr[count] = n;
					 count++;
					 // result_vector.push_back(n);
				 }
			 } else {
				 if(ones > 0) {
					 //allZeros = false;
					 size_t n = 0xc0000000 + ones;
					 ones = 0;
					 result_ptr[count] = n;
					 count++;
					 // result_vector.push_back(n);
				 }
				 if(zeros > 0) {
					 size_t n = 0x80000000 + zeros;
					 zeros = 0;
					 result_ptr[count] = n;
					 count++;
					 // result_vector.push_back(n); //// freq access
				 }
				 //if (num != 0) //allZeros = false;
				 result_ptr[count] = num;
				 count++;
				 // result_vector.push_back(num);//// freq access
			 }
			 word1 = Vector1[++it1];
			 word2 = Vector2[++it2];
		 }
	 }
	 if(ones > 0) {
	 //allZeros = false;
		 size_t n = 0xc0000000 + ones;
		 ones = 0;
		 result_ptr[count] = n;
		 count++;
		 // result_vector.push_back(n);
	 }
	 if(zeros > 0) {
		 size_t n = 0x80000000 + zeros;
		 zeros = 0;
		 result_ptr[count] = n;
		 count++;
		 // result_vector.push_back(n);
	 }



}



int main(int argc, char** argv)
{
    cout<<"Usage: ./proj num_threads\n";
    cout<<"***************************************************************\n";
    int NUM_THREADS_OPENMP =atoi(argv[1]);


    vector<size_t> comp_bitset1;
    vector<size_t> comp_bitset2;

    mybitops Bitops;
    if(DATA_GENERATION)
    {
				cout<<"data generation*\n";
        size_t items = 20000000;
        float fill_percent = .001;
        boost::dynamic_bitset<> tempvector(items);
        cout<<"Data Generation"<<endl;
        for(size_t i=0; i<items*fill_percent; i++) {
            size_t rnd = rand()%items;
            while(tempvector[rnd]==1)
                rnd = rand()%items;
            tempvector[rnd] = 1;
        }

        cout<<"Data Compression"<<endl;
        clock_t t0 = clock();
        comp_bitset1 = Bitops.compressBitset(tempvector);

        clock_t t1 = clock();
        cout<<"compression time:"<< t1-t0<<endl;
        cout<<"vector size:"<< comp_bitset1.size()<<endl;
        comp_bitset2 = comp_bitset1;
        Bitops.save_vector(comp_bitset1,"./vec4", 1000);
    }
    else
    {
        comp_bitset1 = Bitops.load_vector("../data/lvec1");
        comp_bitset2 = Bitops.load_vector("../data/lvec1");
    }

    // clock_t t2 = clock();
    //Bitops.parallel_and(comp_bitset1,comp_bitset2,NUM_THREADS);
    // clock_t t3 = clock();
    // cout<<"parallel_and time:"<< t3-t2<<endl;

    clock_t t4 = clock();
    Bitops.logic_and_ref(&comp_bitset1,&comp_bitset2,0,comp_bitset1.size()-1,0,comp_bitset2.size()-1);
    clock_t t5 = clock();
    cout<<"seq_and time:"<< t5-t4<<endl;
    //pthread_exit(NULL);


		//Lets do it in Cuda version

		//thrust::device_vector

		thrust::device_vector<size_t>  comp_bitset1_device;
		thrust::device_vector<size_t>  comp_bitset2_device;

		comp_bitset1_device = comp_bitset1 ;
		comp_bitset2_device = comp_bitset2 ;


		//size_t* comp_dev_vect1 = thrust::raw_pointer_cast(&comp_bitset1_device) ;
		//size_t* comp_dev_vect2 = thrust::raw_pointer_cast(&comp_bitset2_device) ;


		//lets calculate prefix sum and word array for cuda calculation

		size_t vec1_size = comp_bitset1.size();
		size_t vec2_size = comp_bitset2.size();


		//Create word and prefix sum for vector 1

		size_t* word_lengths1 = new size_t[vec1_size]; //vector of length of the words
		size_t* prefix_sum1 = new size_t[vec1_size]; // prefix-sum generated form word_lengths1
		size_t pre_sum1 = 0;
		size_t vector1_bit_length = 0;

		cudaEvent_t startEventPrefixSum, stopEventPrefixSum;




		//Generate prefix sum and words size array for vector 1
		for(int i = 0 ; i<vec1_size;i++)
		{
			size_t word = comp_bitset1[i];
			int wt = Bitops.word_type(word);
			size_t word_length = 31;
			if(wt != 0)// if a fill word
				word_length = word & 0x3fffffff;
			word_lengths1[i] = word_length;
			prefix_sum1[i] = pre_sum1+word_length;
			pre_sum1 = prefix_sum1[i];
			vector1_bit_length+=word_length;
		}


		//Create word and prefix sum for vector 2

		size_t* word_lengths2 = new size_t[vec2_size]; //vector of length of the words
		size_t* prefix_sum2 = new size_t[vec2_size];   // prefix-sum generated form word_lengths1
		size_t pre_sum2 = 0;
		size_t vector2_bit_length = 0;

		for(int i = 0 ; i<vec2_size;i++)
	  {
	    size_t word = comp_bitset2[i];
	    int wt = Bitops.word_type(word);
	    size_t word_length = 31;
	    if(wt != 0)// if a fill word
	      word_length = word & 0x3fffffff;
	    word_lengths2[i] = word_length;
	    prefix_sum2[i] = pre_sum2+word_length;
	    // printf("%u\n",word_length);
	    pre_sum2 = prefix_sum2[i];
	    vector2_bit_length+=word_length;
	  }


		cudaEvent_t startEvent_kernel, stopEvent_kernel;

		size_t * Vector1 = (size_t*)malloc( vec1_size * sizeof(size_t)) ;
		size_t * Vector2 = (size_t*)malloc( vec2_size * sizeof(size_t)) ;

		size_t outVectorSize = (vec1_size < vec2_size)?vec1_size:vec2_size ;

		size_t * outVector = (size_t*)malloc(outVectorSize * sizeof(size_t) ) ;


		size_t * Vector1_device;
		size_t * Vector2_device;
		size_t * outVector_device;
		size_t * presum2_device ;
		size_t * presum1_device ;
		size_t * word_length_device ;
		size_t * word_length_device2 ;


		std::copy(comp_bitset1.begin(), comp_bitset1.end() , Vector1);
		std::copy(comp_bitset2.begin(), comp_bitset2.end() , Vector2);


		cudaMalloc((void**) &Vector1_device , vec1_size * sizeof(size_t) );
		cudaMalloc((void**) &Vector2_device , vec2_size * sizeof(size_t) );
		cudaMalloc((void**) &outVector_device , outVectorSize * sizeof(size_t)) ;
		cudaMalloc((void**) &presum1_device , vec1_size * sizeof(size_t)) ;
		cudaMalloc((void**) &presum2_device , vec2_size * sizeof(size_t)) ;
		cudaMalloc((void**) &word_length_device , vec1_size * sizeof(size_t)) ;
		cudaMalloc((void**) &word_length_device2 , vec2_size * sizeof(size_t)) ;


		cudaMemcpy(Vector1_device, Vector1 ,vec1_size * sizeof(size_t) , cudaMemcpyHostToDevice ) ;
		cudaMemcpy(Vector2_device, Vector2 ,vec2_size * sizeof(size_t) , cudaMemcpyHostToDevice ) ;
		cudaMemcpy(presum1_device, prefix_sum1 , vec1_size * sizeof(size_t) , cudaMemcpyHostToDevice ) ;
		cudaMemcpy(presum2_device, prefix_sum2 , vec2_size * sizeof(size_t) , cudaMemcpyHostToDevice ) ;
		cudaMemcpy(word_length_device , word_lengths1 , vec1_size * sizeof(size_t) ,  cudaMemcpyHostToDevice  ) ;
		cudaMemcpy(word_length_device2 , word_lengths2 , vec2_size * sizeof(size_t) ,  cudaMemcpyHostToDevice  ) ;



		dim3 dimGrid(10000,1,1);
		dim3 dimBlock(128,1,1);


		cudaEventCreate(&startEvent_kernel);
		cudaEventCreate(&stopEvent_kernel) ;

		cudaEventRecord(startEvent_kernel, 0) ;

		clock_t t_KERNEL1 = clock();

		parallelAndDevice<<<dimGrid, dimBlock>>>(Vector1_device , vec1_size, presum1_device, word_length_device,
		                Vector2_device , vec2_size ,  presum2_device , word_length_device2 , outVector_device, vector1_bit_length , vector2_bit_length) ;

		clock_t t_KERNEL2 = clock();

		cout<<"seq_and time:"<< t5-t4<<endl;

		cudaEventRecord(stopEvent_kernel, 0) ;

		float timeForKernel;
		cudaEventElapsedTime(&timeForKernel, startEvent_kernel, stopEvent_kernel) ;

		printf("  Time for  Kernel : %f\n",  timeForKernel);

		cudaMemcpy(outVector , outVector_device , outVectorSize * sizeof(size_t) , cudaMemcpyDeviceToHost) ;


		cudaFree(Vector1_device);
		cudaFree(Vector2_device);
		cudaFree(outVector_device);
		cudaFree(presum1_device);
		cudaFree(presum2_device);
		cudaFree(word_length_device);
		cudaFree(word_length_device2) ;




    return 0;
}

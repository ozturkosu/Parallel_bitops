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

/*
void save_vector(vector<size_t>& input_vector ,string dir, int repeat_factor)
{
    //first line is the number of words
    // second line is the vector itself
	cout<<"Saving input_vector on "<<dir<<endl;
	ofstream myfile;
	myfile.open(dir);
	if(myfile.is_open())
	{
		myfile<<input_vector.size()*repeat_factor<<endl;
        for(int i = 0;i<repeat_factor;i++)
				{
            for(size_t word : input_vector)
						{
                myfile<<word<<" ";
						}
				}
	}
	else
		cout<<"Could not open file "<<dir<<endl;
	myfile.close();
}
vector<size_t> load_vector(string dir)
{
	////file structure:
    vector<size_t> result_vector;
    cout<<"Loading vector from "<<dir<<endl;

	ifstream myfile;
	myfile.open (dir);
    size_t number_of_words = 0;
    myfile>>number_of_words;
	if(myfile.is_open())
	{
		for(int j = 0;j< number_of_words;j++)
		{
			size_t word = 0;
			myfile>>word;
			result_vector.push_back(word);
		}

	}
	else
	{
		cout<<"Could not open file "<<dir<<endl;
	}
	myfile.close();
    return result_vector;
}

*/
__device__ int word_type(size_t word1){

	if((word1 & 0x80000000) == 0)
		return 0; // literal word
	else{// fill word
		if((word1 & 0xc0000000) == 0xc0000000)
		return 2;//ones
		else
		return 1;//zeros

	}

}

__device__ int ismyfill(size_t word1){

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

//Kernel Method for AND operation
__global__ void parallel_and_kernel(vector<size_t> &vector1, vector<size_t> &vector2)
{
		size_t vec1_size = vector1.size();
		size_t vec2_size = vector2.size();


		//Create word and prefix sum for vector 1

		size_t* word_lengths1 = new size_t[vec1_size]; //vector of length of the words
	  size_t* prefix_sum1 = new size_t[vec2_size]; // prefix-sum generated form word_lengths1
	  size_t pre_sum1 = 0;
	  size_t vector1_bit_length = 0;


		//Generate prefix sum and wors size array for vector 1
		for(int i = 0 ; i<vec1_size;i++)
	  {
	    size_t word = vector1[i];
	    int wt = word_type(word);
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
	    size_t word = vector2[i];
	    int wt = word_type(word);
	    size_t word_length = 31;
	    if(wt != 0)// if a fill word
	      word_length = word & 0x3fffffff;
	    word_lengths2[i] = word_length;
	    prefix_sum2[i] = pre_sum2+word_length;
	    // printf("%u\n",word_length);
	    pre_sum2 = prefix_sum2[i];
	    vector2_bit_length+=word_length;
	  }

		size_t min_bit_length = (vector1_bit_length<vector2_bit_length)?vector1_bit_length:vector2_bit_length;

		size_t *v1_ptr = &vector1[0];
		size_t *v2_ptr = &vector2[0];

		//lets start cuda part
		int tid= blockIdx.x * blockDim.x + threadIdx.x ;

		size_t b1 = tid*CHUNK_SIZE;
		size_t b2 = (tid+1)*CHUNK_SIZE;

		////2.2 calc the word area
		int w11 = myBinarySearchGPU(prefix_sum1,0,vec1_size-1,b1);
		int w12 = myBinarySearchGPU(prefix_sum1,0,vec1_size-1,b2);

		int w21 = myBinarySearchGPU(prefix_sum2,0,vec2_size-1,b1);
		int w22 = myBinarySearchGPU(prefix_sum2,0,vec2_size-1,b2);

		float res_size = ((w12-w11)<(w22-w21))?(w12-w11):(w22-w21);
		size_t min_length = res_size * 1.01;//give 1% more capacity
		vector<size_t> result_vector(min_length);
		size_t *result_ptr = &result_vector[0];


		size_t ones = 0, zeros = 0;
			size_t it1 = w11;
			size_t it2 = w21;
			size_t word1 = vector1[it1];
			size_t word2 = vector2[it2];
			size_t count = 0;
			while(it1 <= w12 && it2 <= w22) {
				if(ismyfill(word1) && ismyfill(word2)) {  //both are fill words
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
						word1 = vector1[++it1];
						word2 = vector2[++it2];
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
						word2 = vector2[++it2];
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
						word1 = vector1[++it1];
					}
				} else if(ismyfill(word1) && !ismyfill(word2)) { //one fill, one literal
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
						word1 = vector1[++it1];
					}
					else {
						if(isone1)
							word1 = 0xc0000000 + (c1 - (size_t)31);
						else
							word1 = 0x80000000 + (c1 - (size_t)31);
					}
					word2 = vector2[++it2];
				} else if(!ismyfill(word1) && ismyfill(word2)) { //one literal, one fill
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
						word2 = vector2[++it2];
					else {
						if(isone2)
							word2 = 0xc0000000 + (c2 - (size_t)31);
						else
							word2 = 0x80000000 + (c2 - (size_t)31);
					}
					word1 = vector1[++it1];
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
					word1 = vector1[++it1];
					word2 = vector2[++it2];
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


__global__ void parallel_and_kernel_V2(vector<size_t> &vector1, vector<size_t> &vector2,size_t * prefixSum)
{


















}

*/

__global__ void parallelAndDevice()
{
	thrust::device_vector<size_t>  comp_bitset1_device;
	thrust::device_vector<size_t>  comp_bitset2_device;




}

int main(int argc, char** argv)
{
    cout<<"Usage: ./proj num_threads\n";
    cout<<"***************************************************************\n";
    int NUM_THREADS =atoi(argv[1]);
    vector<size_t> comp_bitset1;
    vector<size_t> comp_bitset2;

    mybitops Bitops;
    if(DATA_GENERATION)
    {
        size_t items = 10000000;
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
    pthread_exit(NULL);


		//Lets do it in Cuda version

		//thrust::device_vector

		thrust::device_vector<size_t>  comp_bitset1_device;
		thrust::device_vector<size_t>  comp_bitset2_device;








    return 0;
}

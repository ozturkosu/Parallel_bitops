//Project for CUDA

#include <boost/unordered_map.hpp>
#include "mybitops.h"
#include <vector>
#include <unordered_map>
#include <omp.h>
using namespace std;
int NUM_THREADS;
unordered_map<int,vector<size_t>*> results; // key:tid, value:pointer to partial result generated by tid
////###################### Bit Operation ###################################


vector<size_t> mybitops::logic_and_ref(vector<size_t>*( v1),vector<size_t>*( v2), size_t v1begin, size_t  v1end,
                                        size_t v2begin, size_t  v2end ) {
  size_t ones = 0, zeros = 0;
  vector<size_t> result_vector;
  result_vector.reserve(v1end-v1begin);
  size_t it1 = v1begin;
  size_t it2 = v2begin;
  size_t word1 = (*v1)[it1];
  size_t word2 = (*v2)[it2];
  size_t iters = 0;
  clock_t t0 = clock();

  int count = 0;
  /*while(it1 <= v1end && it2 <= v2end) // this does work !
  {
    size_t temp1 = (*v1)[it1];
    size_t temp2 = (*v2)[it2];
    result_vector.push_back(temp1*temp2);
    it1++;
    it2++;
    count++;
  }
  clock_t t4 = clock();
  */
  while(it1 <= v1end && it2 <= v2end) {
    iters++;
    count++;
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
            result_vector.push_back(n);
          }
        } else {
          zeros += c1;
          if(ones > 0) {
            size_t n = 0xc0000000 + ones;
            ones = 0;
            result_vector.push_back(n);
          }
        }
        word1 = (*v1)[++it1];
        word2 = (*v2)[++it2];
      } else if (c1 > c2) {
        if(isone1 && isone2) {
          //allZeros = false;
          ones += c2;
          if(zeros > 0) {
            size_t n = 0x80000000 + zeros;
            zeros = 0;
            result_vector.push_back(n);
          }
        } else {
          zeros += c2;
          if(ones > 0) {
            size_t n = 0xc0000000 + ones;
            ones = 0;
            result_vector.push_back(n);
          }
        }
        if(isone1)
          word1 = 0xc0000000 + (c1 - c2);
        else
          word1 = 0x80000000 + (c1 - c2);
        word2 = (*v2)[++it2];
      } else { //c2 > c1
        if(isone1 && isone2) {
          //allZeros = false;
          ones += c1;
          if(zeros > 0) {
            size_t n = 0x80000000 + zeros;
            zeros = 0;
            result_vector.push_back(n);
          }
        } else {
          zeros += c1;
          if(ones > 0) {
            size_t n = 0xc0000000 + ones;
            ones = 0;
            result_vector.push_back(n);
          }
        }
        if(isone2)
          word2 = 0xc0000000 + (c2 - c1);
        else
          word2 = 0x80000000 + (c2 - c1);
        word1 = (*v1)[++it1];
      }
    } else if(ismyfill(word1) && !ismyfill(word2)) { //one fill, one literal
      size_t c1 = word1 & 0x3fffffff; //lenth
      size_t isone1 = word1 & 0x40000000;  //is one?

      if(isone1) { //if one, depend on literal
        if(ones > 0) {
          size_t n = 0xc0000000 + ones;
          //allZeros = false;
          ones = 0;
          result_vector.push_back(n);
        }
        if(zeros > 0) {
          size_t n = 0x80000000 + zeros;
          zeros = 0;
          result_vector.push_back(n);
        }
        result_vector.push_back(word2);
      } else { //id zero, generate a zero word
        if(ones > 0) {
          size_t n = 0xc0000000 + ones;
          ones = 0;
          result_vector.push_back(n);
        }
        zeros += 31;
      }
      if(c1 == 31){
        word1 = (*v1)[++it1];
      }
      else {
        if(isone1)
          word1 = 0xc0000000 + (c1 - (size_t)31);
        else
          word1 = 0x80000000 + (c1 - (size_t)31);
      }
      word2 = (*v2)[++it2];
    } else if(!ismyfill(word1) && ismyfill(word2)) { //one literal, one fill
      size_t c2 = word2 & 0x3fffffff; //lenth
      size_t isone2 = word2 & 0x40000000;  //is one?

      if(isone2) { //if one, depend on literal
        if(ones > 0) {
          size_t n = 0xc0000000 + ones;
          //allZeros = false;
          ones = 0;
          result_vector.push_back(n);
        }
        if(zeros > 0) {
          size_t n = 0x80000000 + zeros;
          zeros = 0;
          result_vector.push_back(n);
        }
        //if (word1 != 0) //allZeros = false;
        result_vector.push_back(word1);
      } else { //id zero, generate a zero word
        if(ones > 0) {
          size_t n = 0xc0000000 + ones;
          ones = 0;
          result_vector.push_back(n);
        }
        zeros += 31;
      }
      if(c2 == 31)
        word2 = (*v2)[++it2];
      else {
        if(isone2)
          word2 = 0xc0000000 + (c2 - (size_t)31);
        else
          word2 = 0x80000000 + (c2 - (size_t)31);
      }
      word1 = (*v1)[++it1];
    } else { //both are literal words
      size_t num = word1 & word2;
      if(num == 0) {
        zeros += 31;
        if(ones > 0) {
		  //allZeros = false;
          size_t n = 0xc0000000 + ones;
          ones = 0;
          result_vector.push_back(n);
        }
      } else if (num == 0x7fffffff) {
        ones += 31;
        //allZeros = false;
        if(zeros > 0) {
          size_t n = 0x80000000 + zeros;
          zeros = 0;
          result_vector.push_back(n);
        }
      } else {
        if(ones > 0) {
          //allZeros = false;
          size_t n = 0xc0000000 + ones;
          ones = 0;
          result_vector.push_back(n);
        }
        if(zeros > 0) {
          size_t n = 0x80000000 + zeros;
          zeros = 0;
          result_vector.push_back(n);
        }
        //if (num != 0) //allZeros = false;
        result_vector.push_back(num);
      }
      word1 = (*v1)[++it1];
      word2 = (*v2)[++it2];
    }
  }
  if(ones > 0) {
	//allZeros = false;
    size_t n = 0xc0000000 + ones;
    ones = 0;
    result_vector.push_back(n);
  }
  if(zeros > 0) {
    size_t n = 0x80000000 + zeros;
    zeros = 0;
    result_vector.push_back(n);
  }
  clock_t t1 = clock();
  // printf("and %u %u %u\n",v1end-v1begin,v2end-v2begin,iters);
  printf("seq_time: %u,seq_size:%u, count:%u \n",t1-t0,result_vector.size(),count);


  return result_vector;
}
vector<size_t> mybitops::compressBitset(const boost::dynamic_bitset<> dbitset) {
  vector<size_t>cvector;
  size_t zeros = 0;
  size_t ones = 0;

  for(unsigned int j=0; j<dbitset.size();) {
    size_t num = 0;
    for(int k=0; k<31; k++) {
      if(j+k >= dbitset.size())
        break;
      num <<= 1;
      if(dbitset[j+k])
        num++;
    }
    if(j+31 >= dbitset.size()) {
      bool isAllZero = true;
      bool isAllOne = true;
      for(unsigned int k= j; k<dbitset.size(); k++) {
        if(dbitset[k] == true)
          isAllZero = false;
        if(dbitset[k] == false)
          isAllOne = false;
      }
      if(isAllZero) {
        zeros += dbitset.size() - j;
        if(ones > 0) {
          size_t n = 0xc0000000 + ones;
          ones = 0;
          cvector.push_back(n);
        }
        if(zeros > 0) {
          size_t n = 0x80000000 + zeros;
          zeros = 0;
          cvector.push_back(n);
        }
      } else if(isAllOne) {
        ones += dbitset.size() - j;
        if(zeros > 0) {
          size_t n = 0x80000000 + zeros;
          zeros = 0;
          cvector.push_back(n);
        }
        if(ones > 0) {
          size_t n = 0xc0000000 + ones;
          ones = 0;
          cvector.push_back(n);
        }
      } else {
        if(ones > 0) {
          size_t n = 0xc0000000 + ones;
          ones = 0;
          cvector.push_back(n);
        }
        if(zeros > 0) {
           size_t n = 0x80000000 + zeros;
           zeros = 0;
           cvector.push_back(n);
        }
        cvector.push_back(num);
      }
      break;
    }
    j += 31;
    if(num == 0) {
      zeros += 31;
      if(ones > 0) {
        size_t n = 0xc0000000 + ones;
        ones = 0;
        cvector.push_back(n);
      }
    } else if(num == 0x7fffffff) {
      ones += 31;
      if(zeros > 0) {
        size_t n = 0x80000000 + zeros;
        zeros = 0;
        cvector.push_back(n);
      }
    } else {
      if(ones > 0) {
        size_t n = 0xc0000000 + ones;
        ones = 0;
        cvector.push_back(n);
      }
      if(zeros > 0) {
        size_t n = 0x80000000 + zeros;
        zeros = 0;
        cvector.push_back(n);
      }
      cvector.push_back(num);
    }
  } //end j
  return cvector;
}
boost::dynamic_bitset<> mybitops::uncompressIndex(vector<size_t> cvector, size_t uncompressed_size){
  boost::dynamic_bitset<> bitvector(uncompressed_size);
  unsigned int mybit = 0;//current position
  for(vector<size_t>::iterator it=cvector.begin(); it!=cvector.end(); ++it) {
	  if (mybit >= uncompressed_size) break;
    if(((*it) & 0x80000000) == 0) {//if lit
      int temp = 30;
      if(mybit + 31 > uncompressed_size) {
        temp = uncompressed_size - mybit - 1;
      }
      for(int j=temp; j>=0; j--) {
        size_t mask = (size_t)1 << j;
        if(((*it) & mask) > 0)
          bitvector[mybit] = true;
        mybit++;
        if(mybit >= uncompressed_size)
          break;
      }
    } else {
      size_t c = (*it) & 0x3fffffff;
      if(((*it) & 0x40000000) > 0) {  //if all 1
         for(int j=(int)c; j>0; j--) {
           bitvector[mybit] = true;
           mybit++;
           if(mybit >= uncompressed_size)
             break;
         }
      } else  //if all 0
         mybit += (int)c;
    }
  } //end iterator
  return bitvector;
}

int mybitops::ismyfill(size_t word1) {
  if((word1 & 0x80000000) == 0)
    return 0; // literal word
  else
    return 1; // fill word
}
int ismyfill(size_t word1) {
  if((word1 & 0x80000000) == 0)
    return 0; // literal word
  else
    return 1; // fill word
}
/*
 * Returns the word type:
 * 0: literal
 * 1: zeros
 * 2: ones
 */

int mybitops::word_type(size_t word1) {
  if((word1 & 0x80000000) == 0)
    return 0; // literal word
  else{// fill word
	  if((word1 & 0xc0000000) == 0xc0000000)
		return 2;//ones
	  else
		return 1;//zeros

	}
}
////#################################### New Code ##############################################

size_t myBinarySearch (size_t* arr, size_t l, size_t r, size_t x)
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
      return myBinarySearch(arr, l, mid-1, x);
    // Else the element can only be present
    // in right subarray
    return myBinarySearch(arr, mid+1, r, x);
  }

  // We reach here when element is not
  // present in array
  if(r<0)
    return 0;
  return r;
}
vector<size_t> mybitops::parallel_and(vector<size_t> &vector1, vector<size_t> &vector2, int n_threads)
{
  ////@@@@@@@@@@@@@@@@@@@@@@@@@@@@ serial preprocessing phase @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  ////@@@@@ calculate size_vector and prefix_sum vector for each input vector
  NUM_THREADS = n_threads;
  // clock_t t1 = clock();
  size_t vec1_size = vector1.size();
  size_t vec2_size = vector2.size();
  // printf("vec1Size: %u vec2Size: %u\n",vec1_size,vec2_size);
  size_t* word_lengths1 = new size_t[vec1_size]; //vector of length of the words
  size_t* prefix_sum1 = new size_t[vec2_size]; // prefix-sum generated form word_lengths1
  size_t pre_sum1 = 0;
  size_t vector1_bit_length = 0;

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
  // printf("%u,%u,%u\n",vector1_bit_length,vector2_bit_length,min_bit_length);

  // clock_t t2 = clock();
  // cout<<"preprocessing time:"<< t2-t1<<endl;

  ////@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ multi threading @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    // Request threads
    omp_set_num_threads(NUM_THREADS);

    size_t *v1_ptr = &vector1[0];
    size_t *v2_ptr = &vector2[0];

    #pragma  omp parallel //num_threads(10)
    {
    size_t actual_num_threads = omp_get_num_threads();

    ////@@@@@@@@@@@@@@@@@@@@@@ 1. get the thread info @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
      int tid;
      tid = omp_get_thread_num();// query the current thread id
      // printf("tid:%d\n",tid);

        // printf("Assigned Threads:%d\n",actual_num_threads);
    ////@@@@@@@@@@@@@@@@@@@@@@ 2. compute the working area @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
      size_t chunk_size = min_bit_length/actual_num_threads;
      // printf("A-len:%u, CS:%u,tn:%u\n",min_bit_length,chunk_size,actual_num_threads);
      ////2.1 calc the bit area
      size_t b1 = tid*chunk_size;
      size_t b2 = (tid+1)*chunk_size;

      ////2.2 calc the word area
      int w11 = myBinarySearch(prefix_sum1,0,vec1_size-1,b1);
      int w12 = myBinarySearch(prefix_sum1,0,vec1_size-1,b2);

      int w21 = myBinarySearch(prefix_sum2,0,vec2_size-1,b1);
      int w22 = myBinarySearch(prefix_sum2,0,vec2_size-1,b2);

      // printf("tid:%d b1:%u b2:%u w11:%u w12:%u w21:%u w22:%u\n",tid,b1,b2,w11,w12,w21,w22);
      // printf("tid:%d w11:%u w12:%u w21:%u w22:%u\n",tid,w11,w12,w21,w22);

    ////@@@@@@@@@@@@@@@@@@@@@@ 3. compute the AND for your area @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
      ////4.1  TODO: right alignment

      float res_size = ((w12-w11)<(w22-w21))?(w12-w11):(w22-w21);
      size_t min_length = res_size * 1.01;//give 1% more capacity
      vector<size_t> result_vector(min_length);
      size_t *result_ptr = &result_vector[0];
      // result_vector.reserve(min_length);
      // vector<size_t>* myResult = new vector<size_t>;
      ////4.2 sequential_and my working area
      ////***************************************** AND *****************************************************
      // *myResult = Bitops.logic_and_ref(vec1,vec2,w11,w12,w21,w22);

      /*size_t it1 = w11;
      size_t it2 = w21;
      int count = 0;
      while(it1 <= w12 && it2 <= w22) // this does work !
      {
        size_t temp1 = v1_ptr[it1];
        size_t temp2 = v2_ptr[it2];
        result_ptr[count]=(temp1*temp2);
        it1++;
        it2++;
        count++;
      }*/
      clock_t t3 = clock();

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

      // printf("(%u,%u:%u)\n",tid,w12-w11,w22-w21);

      // printf("(%u,%u,res:%u,time:%u)\n",tid,result_vector.size(),min_length,t4-t3);//,w11,w12,w21,w22);
      ////****************************************************************************************************
      clock_t t4 = clock();

      printf("par_time%u:%u,par_size:%u, count:%u \n",tid,t4-t3,result_vector.size(), count);//,t2-t1,w12-w11,w22-w21);
      ////4.1 add my results to the global map
      // results.insert({tid,myResult});
      // if(tid!=0)
    }// joins :)

    // cout<<"multi-threading time:"<< t4-t3<<endl;
    // clock_t t5 = clock();
    /*
    ////@@@@@ Merge the the partial results
    // result_vector.reserve( A.size() + B.size() ); // preallocate memory
    for(int i = 0;i<NUM_THREADS;i++)
      result_vector.insert(result_vector.end(), results[i]->begin(), results[i]->end());

    clock_t t6 = clock();
    cout<<"merge time:"<< t4-t3<<endl;  */
    vector<size_t> XX;

  return XX;
}


void mybitops::save_vector(vector<size_t>& input_vector ,string dir, int repeat_factor)
{
  //first line is the number of words
  // second line is the vector itself
  cout<<"Saving input_vector on "<<dir<<endl;
  ofstream myfile;
  myfile.open (dir);
  if(myfile.is_open())
  {
    myfile<<input_vector.size()*repeat_factor<<endl;
        for(int i = 0;i<repeat_factor;i++)
            for(size_t word : input_vector)
                myfile<<word<<" ";
  }
  else
    cout<<"Could not open file "<<dir<<endl;
  myfile.close();

}

vector<size_t> mybitops::load_vector(string dir)
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

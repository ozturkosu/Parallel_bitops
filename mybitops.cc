#include <iostream>
#include <boost/unordered_map.hpp>
#include "mybitops.h"
#include <vector>
using namespace std;
int NUM_THREADS = 4;
static pthread_barrier_t barrier1;
struct th_arg{
//pair<long,long> * arguments;
long  tid;
};
////###################### Bit Operation ###################################


 vector<size_t> mybitops::logic_and(vector<size_t> vector1, vector<size_t> vector2) {
  int ones = 0, zeros = 0;
  vector<size_t> result_vector;
  vector<size_t>::iterator it1 = vector1.begin();
  vector<size_t>::iterator it2 = vector2.begin();
  size_t word1 = (*it1);
  size_t word2 = (*it2);

 // bool //allZeros = true;

  while(it1 != vector1.end() && it2 != vector2.end()) {
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
        word1 = *(++it1);
        word2 = *(++it2);
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
        word2 = *(++it2);
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
        word1 = *(++it1);
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
        //if (word2 != 0) //allZeros = false;
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
        word1 = *(++it1);
      }
      else {
        if(isone1)
          word1 = 0xc0000000 + (c1 - (size_t)31);
        else
          word1 = 0x80000000 + (c1 - (size_t)31);
      }
      word2 = *(++it2);
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
        if (word1 != 0) //allZeros = false;
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
        word2 = *(++it2);
      else {
        if(isone2)
          word2 = 0xc0000000 + (c2 - (size_t)31);
        else
          word2 = 0x80000000 + (c2 - (size_t)31);
      }
      word1 = *(++it1);
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
        if (num != 0) //allZeros = false;
        result_vector.push_back(num);
      }
      word1 = *(++it1);
      word2 = *(++it2);
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

  return result_vector;
}

 vector<size_t> mybitops::logic_and_ref(vector<size_t> &vector1, vector<size_t> &vector2) {
  int ones = 0, zeros = 0;
  vector<size_t> result_vector;
  vector<size_t>::iterator it1 = vector1.begin();
  vector<size_t>::iterator it2 = vector2.begin();
  size_t word1 = (*it1);
  size_t word2 = (*it2);

  //bool //allZeros = true;
  while(it1 != vector1.end() && it2 != vector2.end()) {
    
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
        word1 = *(++it1);
        word2 = *(++it2);
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
        word2 = *(++it2);
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
        word1 = *(++it1);
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
        //if (word2 != 0) //allZeros = false;
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
        word1 = *(++it1);
      }
      else {
        if(isone1)
          word1 = 0xc0000000 + (c1 - (size_t)31);
        else
          word1 = 0x80000000 + (c1 - (size_t)31);
      }
      word2 = *(++it2);
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
        word2 = *(++it2);
      else {
        if(isone2)
          word2 = 0xc0000000 + (c2 - (size_t)31);
        else
          word2 = 0x80000000 + (c2 - (size_t)31);
      }
      word1 = *(++it1);
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
      word1 = *(++it1);
      word2 = *(++it2);
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
	//cout<<vector1.size()<<"  "<<vector1.size()<<" it1"<<it1<<"  it2:"<<it2<<endl;
  return result_vector;
}
/*
 * Part of the WAH algorithm
 * Check if the current word is a fill word or a literal word
 */ 

vector<size_t> mybitops::logic_or(vector<size_t> vector1, vector<size_t> vector2) {
  int ones = 0, zeros = 0;
  vector<size_t> result_vector;
  vector<size_t>::iterator it1 = vector1.begin();
  vector<size_t>::iterator it2 = vector2.begin();
  size_t word1 = (*it1);
  size_t word2 = (*it2);
  while(it1 != vector1.end() && it2 != vector2.end()) {
    if(ismyfill(word1) && ismyfill(word2)) {  //both are fill words
      size_t c1 = word1 & 0x3fffffff; //get the len of fill word
      size_t c2 = word2 & 0x3fffffff; //get the len of fill word
      size_t isone1 = word1 & 0x40000000; //judge if it is one
      size_t isone2 = word2 & 0x40000000; //judge if it is one
      if(c1 == c2) {
        if(isone1 || isone2) {
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
        word1 = *(++it1);
        word2 = *(++it2);
      } else if (c1 > c2) {
        if(isone1 || isone2) {
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
        word2 = *(++it2);
      } else { //c2 > c1
        if(isone1 || isone2) {
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
        word1 = *(++it1);
      }
    } else if(ismyfill(word1) && !ismyfill(word2)) { //one fill, one literal
      size_t c1 = word1 & 0x3fffffff; //lenth
      size_t isone1 = word1 & 0x40000000;  //is one?
      if(!isone1) { //if zero, depend on literal
        if(ones > 0) {
          size_t n = 0xc0000000 + ones;
          ones = 0;
          result_vector.push_back(n);
        }
        if(zeros > 0) {
          size_t n = 0x80000000 + zeros;
          zeros = 0;
          result_vector.push_back(n);
        }
        result_vector.push_back(word2);
      } else { //if one, generate a one word
        if(zeros > 0) {
          size_t n = 0x80000000 + zeros;
          zeros = 0;
          result_vector.push_back(n);
        }
        ones += 31;
      }
      if(c1 == 31)
        word1 = *(++it1);
      else {
        if(isone1)
          word1 = 0xc0000000 + (c1 - (size_t)31);
        else
          word1 = 0x80000000 + (c1 - (size_t)31);
      }
      word2 = *(++it2);
    } else if(!ismyfill(word1) && ismyfill(word2)) { //one literal, one fill
      size_t c2 = word2 & 0x3fffffff; //lenth
      size_t isone2 = word2 & 0x40000000;  //is one?

      if(!isone2) { //if one, depend on literal
        if(ones > 0) {
          size_t n = 0xc0000000 + ones;
          ones = 0;
          result_vector.push_back(n);
        }
        if(zeros > 0) {
          size_t n = 0x80000000 + zeros;
          zeros = 0;
          result_vector.push_back(n);
        }
        result_vector.push_back(word1);
      } else { //if one, generate a one word
        if(zeros > 0) {
          size_t n = 0xc0000000 + zeros;
          zeros = 0;
          result_vector.push_back(n);
        }
        ones += 31;
      }
      if(c2 == 31)
        word2 = *(++it2);
      else {
        if(isone2)
          word2 = 0xc0000000 + (c2 - (size_t)31);
        else
          word2 = 0x80000000 + (c2 - (size_t)31);
      }
      word1 = *(++it1);
    } else { //both are literal words
      size_t num = word1 | word2;
      if(num == 0) {
        zeros += 31;
        if(ones > 0) {
          size_t n = 0xc0000000 + ones;
          ones = 0;
          result_vector.push_back(n);
        }
      } else if (num == 0x7fffffff) {
        ones += 31;
        if(zeros > 0) {
          size_t n = 0x80000000 + zeros;
          zeros = 0;
          result_vector.push_back(n);
        }
      } else {
        if(ones > 0) {
          size_t n = 0xc0000000 + ones;
          ones = 0;
          result_vector.push_back(n);
        }
        if(zeros > 0) {
          size_t n = 0x80000000 + zeros;
          zeros = 0;
          result_vector.push_back(n);
        }
        result_vector.push_back(num);
      }
      word1 = *(++it1);
      word2 = *(++it2);
    }
  }
  if(ones > 0) {
    size_t n = 0xc0000000 + ones;
    ones = 0;
    result_vector.push_back(n);
  }
  if(zeros > 0) {
    size_t n = 0x80000000 + zeros;
    zeros = 0;
    result_vector.push_back(n);
  }
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
// void mybitops::print_compressed(vector<size_t>comp_bitset )
// {
//     for( size_t i = 0; i<comp_bitset.size();i++)
//     {
//         size_t word = comp_bitset[i];
//         if(Bitops.word_type(word)==0)
//             cout<<"L ";//0x"<<hex<<word<<dec<<" ";
//         else if(Bitops.word_type(word)==1)
//             cout<<"Z"<<(word & 0x3fffffff)<<" ";
//         else
//             cout<<"O"<<(word & 0x3fffffff)<<" ";
//     }
//     cout<<endl;
// }
void* thread_kernel(void* args)
{
  th_arg *thread_arg = (th_arg *)args;
  int tid = thread_arg->tid;
  cout<<"Thread number\n"<<tid<<endl;
}
void mybitops::parallel_and()//vector<size_t> &vector1, vector<size_t> &vector2)
{
  //######################## serial preprocessing phase #############################
  //###### 1. calculate size_vector and prefix_sum vector for each input vector 
 /* 
  size_t* word_lengths1 = new size_t[vector1.size()]; 
  size_t* prefix_sum1 = new size_t[vector1.size()]; 
  size_t pre_sum1 = 0;
  size_t vector1_bit_length = 0;

  for(int i = 0 ; i<vector1.size();i++)
  {
    size_t word = vector1[i];
    int wt = word_type(word);
    int word_length = 31;
    if(wt != 0)// if a fill word
      word_length = word & 0x3fffffff;
    word_lengths1[i] = word_length;
    prefix_sum1[i] = pre_sum1+word_length;
    pre_sum1 = prefix_sum1[i];
    vector1_bit_length+=word_length;
  }
  size_t* word_lengths2 = new size_t[vector2.size()]; 
  size_t* prefix_sum2 = new size_t[vector2.size()];   
  size_t pre_sum2 = 0;
  size_t vector2_bit_leng th = 0;  
  for(int i = 0 ; i<vector2.size();i++)
  {
    size_t word = vector2[i];
    int wt = word_type(word);
    int word_length = 31;
    if(wt != 0)// if a fill word
      word_length = word & 0x3fffffff;
    word_lengths2[i] = word_length;
    prefix_sum2[i] = pre_sum2+word_length;
    pre_sum2 = prefix_sum2[i];
    vector2_bit_length+=word_length;    
  }
  int min_bit_length = (vector1_bit_length<vector2_bit_length)?vector1_bit_length:vector2_bit_length;
*/  
  //####### 2. multi threading
  pthread_t *threads;
  threads = new pthread_t[NUM_THREADS];
  void *th_status;
  int rc;
  for(int i=0; i < NUM_THREADS; i++)
  {
    th_arg *thread_args;
		thread_args = new th_arg;
    thread_args->tid = i;
    rc = pthread_create(&threads[i], NULL,thread_kernel, (void*) thread_args);
    if(rc)
    {
      cout<<"ERROR; return code from pthread_create() is "<< rc<<endl;
      exit(-1);
    }
  }

  for(int j=0; j < NUM_THREADS; j++)
  {
    rc = pthread_join( threads[j], NULL); 
    if(rc)
    {
      cout<<"ERROR; return code from pthread_join() is "<< rc<<endl;
      exit(-1);
    }
  }
}

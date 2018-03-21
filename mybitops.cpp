#include <iostream>
#include <boost/unordered_map.hpp>
#include <boost/dynamic_bitset.hpp>
#include "mybitops.h"
#include <vector>
using namespace std;
////###################### Bit Operation ###################################


 vector<size_t> mybitops::logic_and(vector<size_t> vector1, vector<size_t> vector2) {
  int ones = 0, zeros = 0;
  vector<size_t> result_vector;
  vector<size_t>::iterator it1 = vector1.begin();
  vector<size_t>::iterator it2 = vector2.begin();
  size_t word1 = (*it1);
  size_t word2 = (*it2);

  bool allZeros = true;

  while(it1 != vector1.end() && it2 != vector2.end()) {
    if(ismyfill(word1) && ismyfill(word2)) {  //both are fill words
      size_t c1 = word1 & 0x3fffffff;         //get the len of fill word
      size_t c2 = word2 & 0x3fffffff;         //get the len of fill word
      size_t isone1 = word1 & 0x40000000;     //check if it is one
      size_t isone2 = word2 & 0x40000000;     //check if it is one

      if(c1 == c2) {
        if(isone1 && isone2) {
          allZeros = false;
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
          allZeros = false;
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
          allZeros = false;
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
          allZeros = false;
          ones = 0;
          result_vector.push_back(n);
        }
        if(zeros > 0) {
          size_t n = 0x80000000 + zeros;
          zeros = 0;
          result_vector.push_back(n);
        }
        if (word2 != 0) allZeros = false;
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
          allZeros = false;
          ones = 0;
          result_vector.push_back(n);
        }
        if(zeros > 0) {
          size_t n = 0x80000000 + zeros;
          zeros = 0;
          result_vector.push_back(n);
        }
        if (word1 != 0) allZeros = false;
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
		  allZeros = false;
          size_t n = 0xc0000000 + ones;
          ones = 0;
          result_vector.push_back(n);
        }
      } else if (num == 0x7fffffff) {
        ones += 31;
        allZeros = false;
        if(zeros > 0) {
          size_t n = 0x80000000 + zeros;
          zeros = 0;
          result_vector.push_back(n);
        }
      } else {
        if(ones > 0) {
          allZeros = false;
          size_t n = 0xc0000000 + ones;
          ones = 0;
          result_vector.push_back(n);
        }
        if(zeros > 0) {
          size_t n = 0x80000000 + zeros;
          zeros = 0;
          result_vector.push_back(n);
        }
        if (num != 0) allZeros = false;
        result_vector.push_back(num);
      }
      word1 = *(++it1);
      word2 = *(++it2);
    }
  }
  if(ones > 0) {
	allZeros = false;
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

  bool allZeros = true;
  while(it1 != vector1.end() && it2 != vector2.end()) {
    
    if(ismyfill(word1) && ismyfill(word2)) {  //both are fill words
      size_t c1 = word1 & 0x3fffffff;         //get the len of fill word
      size_t c2 = word2 & 0x3fffffff;         //get the len of fill word
      size_t isone1 = word1 & 0x40000000;     //check if it is one
      size_t isone2 = word2 & 0x40000000;     //check if it is one

      if(c1 == c2) {
        if(isone1 && isone2) {
          allZeros = false;
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
          allZeros = false;
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
          allZeros = false;
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
          allZeros = false;
          ones = 0;
          result_vector.push_back(n);
        }
        if(zeros > 0) {
          size_t n = 0x80000000 + zeros;
          zeros = 0;
          result_vector.push_back(n);
        }
        if (word2 != 0) allZeros = false;
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
          allZeros = false;
          ones = 0;
          result_vector.push_back(n);
        }
        if(zeros > 0) {
          size_t n = 0x80000000 + zeros;
          zeros = 0;
          result_vector.push_back(n);
        }
        if (word1 != 0) allZeros = false;
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
		  allZeros = false;
          size_t n = 0xc0000000 + ones;
          ones = 0;
          result_vector.push_back(n);
        }
      } else if (num == 0x7fffffff) {
        ones += 31;
        allZeros = false;
        if(zeros > 0) {
          size_t n = 0x80000000 + zeros;
          zeros = 0;
          result_vector.push_back(n);
        }
      } else {
        if(ones > 0) {
          allZeros = false;
          size_t n = 0xc0000000 + ones;
          ones = 0;
          result_vector.push_back(n);
        }
        if(zeros > 0) {
          size_t n = 0x80000000 + zeros;
          zeros = 0;
          result_vector.push_back(n);
        }
        if (num != 0) allZeros = false;
        result_vector.push_back(num);
      }
      word1 = *(++it1);
      word2 = *(++it2);
    }
  }
  if(ones > 0) {
	allZeros = false;
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


boost::dynamic_bitset<> mybitops::uncompressIndex(vector<size_t> cvector, unsigned long uncompressed_size){
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
/*
 * counts the number of ones up to position i inconclusive*/

int mybitops::CountOnesUpto(vector<size_t> &compressed_vector, int position)
{
clock_t tq = clock();

	int current_position = 0;
	int ones_count = 0;
	/* main loop checks each and every word and calculates
	 * the number of ones based on the type of the word it 
	 * will break if the position reached.
	 */
	int it = 0;
	for(auto word: compressed_vector)	                         
	{
		it++;
		int wordType = word_type(word);

			if(wordType==0)// if the word is literal
			{
				//cout<<"lit "<<hex<<word<<endl;
				boost::dynamic_bitset<> word_bits(32, word); // convert word to bits TODO: check performance might be bottle neck
				int i = 30;
				if(it == compressed_vector.size())
					i = position - current_position-1;
				bool flag = false;
				for(int j = i;j>=0;j--)// check up to the end of the array or if finished
				{					
					//cout<<dec<<i<<endl;
					if(word_bits.test(j))//if word[i] == 1 then 
					{
						ones_count++;
						//flag = true;
					}
					//cout<<dec<<current_position<<":" << position<<":"<<ones_count<<"   "<<word_bits.test(i)<<"\n";
					//if(flag)
					current_position++;
					
					if(current_position >= position)
					{
						//cout<<"returned\n";
						return ones_count;
					}				
				}
			}
			else if (wordType==1) // if the word is zeros fill word
			{
				//cout<<"zeros "<<hex<<word<<endl;
				int word_length = word & 0x3fffffff; // get the # of consecutive 0s
				if((current_position+word_length)>=position)
				{
					//current_position = position;
					return ones_count;
				}
				else
				{
					current_position += word_length;
				}
		
			}
			else if (wordType==2)// if the word is ones fill word
			{
				//cout<<"ones "<<hex<<word<<endl;
				int word_length = word & 0x3fffffff; // get the # of consecutive 1s
				if((current_position+word_length)>=position)
				{
					return ones_count+(position - current_position);
				}
				else
				{
					current_position += word_length;
					ones_count+=word_length;
				}

			}
	}
	return ones_count;
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

void mybitops::test_count()
{
	boost::dynamic_bitset<> test(64, 0xabcd007677aaaab7 );
	cout<<"uncomp:"<<test<<endl;
	vector<size_t> comp_test = compressBitset(test);

	//shift based
	cout<<"\nshiftbased:\n";
	for(int i =0;i<64;i++)
	{
		boost::dynamic_bitset<> shifted_bit_representator;
		int shifts = test.size()-i-1;
		shifted_bit_representator = test<<(shifts);
		int ones = shifted_bit_representator.count();	
		cout<<ones<<",";
	}
	//fast
	cout<<"\nfast:\n";
	for(int i =0;i<64;i++)
	{
		cout<<dec<<CountOnesUpto(comp_test, i)<<",";
		//assert(false);
	}
	cout<<"\n";

}

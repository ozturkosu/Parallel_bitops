#ifndef MYBITOPS_H
#define MYBITOPS_H
#include <iostream>
#include <vector>
#include <boost/dynamic_bitset.hpp> 
using namespace std;
class mybitops{
	public:
	mybitops(){}
	~mybitops(){}
	////###################### Bit Operation ###################################
/*
	 *Perform logic AND operation directly based on compressed bitvectors 
	 */
	 vector<size_t> logic_and_ref(vector<size_t>* v1,vector<size_t>* v2, size_t v1begin, size_t  v1end,
                                        size_t v2begin, size_t  v2end );
	/*
	 * Use WAH compression algorithm to compress one bitvector
	 * note: randomness increase the compression time
	 */ 
	vector<size_t> compressBitset(const boost::dynamic_bitset<> dbitset);
	/*
	 * Use WAH decompression algorithm to decompress all the bitvector
	 */ 
	boost::dynamic_bitset<> uncompressIndex(vector<size_t> cvector, size_t uncompressed_size);
	 int ismyfill(size_t word1);
	 /*
	  * return the word type
	  * 0:
	  * 1:
	  * 2:
	  */
	 int word_type(size_t word1);
	vector<size_t> parallel_and(vector<size_t> &vector1, vector<size_t> &vector2);	
};
#endif

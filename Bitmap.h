#ifndef BITMAP_H_
#define BITMAP_H_
#include <map>
#include <iostream>
#include <typeinfo>
#include <stdio.h>
#include <sys/time.h>
#include <boost/unordered_map.hpp>
#include <boost/dynamic_bitset.hpp>
#include <unordered_map>
#include "mybitops.h"
#include <vector>

#define BIN_INTERVAL 1
using namespace std;
/* Noise Value should be smaller than this value, only for ocean data */
//#define NOISE_VALUE_LOW  -100000000000
//#define NOISE_VALUE_HIGH 1000000000000
/* High-level indices bin inteval */


/* struct to keep the lowerbound and upperbound of index bins */
typedef struct index_bin{
  int min_val;
  int max_val;
} index_bin;
/* the struct to keep track of the statistics of each bitvector*/
struct stat{
	int sum;
	int count;
	int min;
	int max;
};


template <class a_type>
class Bitmap {
private:
////################################ containers #############################################
	float numpres;//numpres: The precision(number of low-level bins) of current variable==> a scaling factor
	a_type minvalue,maxvalue;// minvalue and max value of the whole bitmap; 
	unsigned long itemsCount;// number of items in the bitmap
	map<int,int>* varvalmap;
	vector<struct index_bin>* firstlevelvalue;// the bin boundaries (values are multiplied by numpres)
	vector<vector<size_t>>* firstlevelvectors;// high-level bins
	vector<vector<size_t>>* secondlevelvectors;//low-level bins
	
	//================ pre-aggregation statistics
	vector<stat> second_level_statistics; // stats of each second level bin
	vector<stat> first_level_statistics;//stats of each first level bin
	unordered_map<int,int>* second_level_sums; //sum of each second level bin
	unordered_map<int,int>* first_level_sums;//sum of each first level bin
	//=================chunked bitvectors ============
	// vector<vector<vector<size_t>>> * first_level_bitmap;// high-level bitmap
	// vector<vector<vector<size_t>>> * second_level_bitmap;// low-level bitmap
	// int chunk_size = 100;
	//===================== accesss to bit operations ==================
	mybitops Bitops;
	
////#############################  private functions #####################################	
	/*
	 * Set precision(number of bins) of current indexing based on value ranges
	 * In current solution, we control the total number of bins between [100, 999].
	 * Users can modify this code to generate flexible bin numbers
	 * In the future, we plan to provide an interface for users to specify the bin number
	 */ 
	void setPrecision();
	void calcPreAgg();
	
////############################# save/load functions ###################################
void save_bitmap(string dir);
void load_bitmap(string dir); 
void save_variables(string dir);
void save_secondlevelvectors(string dir);
void save_firstlevelvectors(string dir);
void save_firstlevelvalue(string dir);
void save_second_level_statistics(string dir);
void save_first_level_statistics(string dir);
void save_second_level_sums(string dir);
void save_first_level_sums(string dir);

void load_variables(string dir);	
void load_secondlevelvectors(string dir);
void load_firstlevelvectors(string dir);
void load_firstlevelvalue(string dir);
void load_second_level_statistics(string dir);
void load_first_level_statistics(string dir);
// void load_second_level_sums(string dir);
// void load_first_level_sums(string dir);

public:
	
	Bitmap(string dir,a_type* array, unsigned long items);
	Bitmap(string dir);
	~Bitmap();	
	/*making the pre-aggregation statistics*/
	////########################### gets ######################################
	float get_first_level_sum(int binNumber);
	float get_first_level_count(int binNumber);
	float get_second_level_sum(int binNumber);
	float get_second_level_count(int binNumber);
	float get_numpres();
	vector<size_t> get_firstlevelvector(int binNumber);
	vector<struct index_bin>* get_firstlevelvalue();
	/* returns the value of the node 
	 * input: index
	 * output: the bin <min,max> values as a pair<int,int>; returns NULL if the value is not in none of the bins
	 */
	pair<int,int>* get_value(int index);
	
	/*
	 * checks if an index exist in a bin*/
	// bool isInBin(int bin_number,int chunk_number, vector<size_t>& comp_chunk_index); 
	
	/*
	 * Find the cardinality of high-level bitmap indices (or number of high-level bins)
	 */	
	unsigned long getL1Size();
	
	/*
 	* Find the cardinality of low-level bitmap indices(or number of low-level bins)
 	*/
	unsigned long getL2Size();
	
	/*get the bitmap maxval*/
	int get_max();
	
	/*get the bitmap minval*/
	int get_min();
	
	/*returns the itemCounts*/
	long get_count();
	
	////########################### prints ###################################
	void print_second_level_sums();
	void print_min_max();
	
	/*returns the first level bitvectors*/
 	void print_first_level_uncompressed_vectors();
	
	void print_varvalmap();
	
	/*returns the second level bitvectors*/
 	void print_second_level_uncompressed_vectors();
	
	/*prints the pre-aggregation statistics*/
	void print_stat();

};
#endif
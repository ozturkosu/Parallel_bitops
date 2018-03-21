#include "Bitmap.h"
#include <vector>
#include <cmath>
#include <fstream>

using namespace std;
#define DebugMode false// generate more text
#define TRACE false // to debug the node number to child bitmap tree index
#define SET_PREC false // set_precision()
/*
 * the constructor function
 */
template <class a_type> 
Bitmap<a_type>::Bitmap(string dir,a_type* array, unsigned long items){
	clock_t t0 = clock();
	itemsCount = 0;// initially set the number of the items to 0
	firstlevelvalue = new vector<struct index_bin>();// a vector of the the min and max values of the first level bins
	varvalmap = new map<int,int>();
	second_level_sums = new unordered_map<int,int>();
	first_level_sums = new unordered_map<int,int>();
	firstlevelvectors = new vector<vector<size_t>>();// high-level indices
	secondlevelvectors = new vector<vector<size_t>>();//low-level indecies 
	//####################### chunked bitmap ###########
	// second_level_bitmap = new vector<vector<vector<size_t>>>();
	// first_level_bitmap = new vector<vector<vector<size_t>>>();
	//##################################################
	clock_t t1 = clock();
	cout<< t1-t0<<endl;
	//fetch the data block and initialize vector size
	a_type * data_in = array;
	//====== calculate the minimum and the maximum of the data (using a linear search)
	minvalue = data_in[0];
	maxvalue = data_in[0];
	for(int i=0; i<items; i++) {
		//if(data_in[i] < NOISE_VALUE_HIGH &&
		//  (maxvalue >= NOISE_VALUE_HIGH ||  data_in[i] > maxvalue))
		if(data_in[i] > maxvalue)
			maxvalue = data_in[i];
		//if(data_in[i] > NOISE_VALUE_LOW &&
		//  (minvalue <= NOISE_VALUE_LOW ||data_in[i] < minvalue))
		if(data_in[i] < minvalue)
			minvalue = data_in[i];
	}
	clock_t t2 = clock();
	cout<< t2-t1<<endl;
	//set the current precision(total number of low-level bins) based on value ranges
	setPrecision();
	
	//Generate the dintinct value group for each variable
	
	int count = 0;	
	for(int i=0; i<items; i++) {
	//if(data_in[i] > NOISE_VALUE_LOW && data_in[i] < NOISE_VALUE_HIGH) {
		int temp = (int)round((data_in[i] * numpres));
		varvalmap->insert(pair<int, int>(temp, -1));
	//	}
	}
	for(map<int,int>::iterator it=varvalmap->begin(); it!=varvalmap->end(); it++) 
		(*it).second = count++;

	vector<vector<int> > bitvectors(varvalmap->size());
	for(int i=0; i<items; i++) {
		//if(data_in[i] > NOISE_VALUE_LOW && data_in[i] < NOISE_VALUE_HIGH) {
		  int temp = (int)round((data_in[i] * numpres));
		  bitvectors[varvalmap->find(temp)->second].push_back(i);
		//}
	}
	// this part take a lot of time
	clock_t t3 = clock();
	cout<< t3-t2<<endl;
	//============== calculate second_level_sums
	for(auto elem: *varvalmap) {
		int scaled_data = elem.first;
		int bin_number = elem.second;
		int data = scaled_data/numpres;
		int bitvector_size = bitvectors[bin_number].size();
		second_level_sums->insert(pair<int, int>(bin_number,data*bitvector_size));
	}
//////////////////////Generate the second-level(low-level) bitvectors/////////////////////
	clock_t t4 = clock();
	cout<< t4-t3<<endl;
	for(int i=0; i<varvalmap->size(); i++) {
		boost::dynamic_bitset<> tempvector(items);//////////////
		cout<<bitvectors[i].size()<<endl;
		for(int j=0; j<bitvectors[i].size(); j++) {
		  tempvector[bitvectors[i][j]] = 1;
		}
		cout<<"here compress"<<endl;

		secondlevelvectors->push_back(Bitops.compressBitset(tempvector));
	}
	clock_t t5 = clock();
	cout<< t5-t4<<endl;
	/*int i=0;
	for(auto bitvector:(*second_level_bitmap))
	{
		cout<<"bitvector "<<i<<endl;
		int j=0;
		for(auto chunk : bitvector)
		{
			cout<<"chunk"<<j<<":"<<chunk.size()<<" ";
			j++;
		}
		cout<<endl;
		i++;
	}*/
	
////////////////////////Generate the first-level(high-level) bitvectors////////////////////
	map<int,int>::iterator curit=varvalmap->begin();
	map<int,int>::iterator preit;
	int k = 0;

	for(int i=0; i<varvalmap->size(); ) {
		struct index_bin curbin;
		curbin.min_val = (*curit).first;
		preit = curit++;
		int sum = (*second_level_sums)[i];//===
		vector<size_t> firstlevelidx = (*secondlevelvectors)[i++];
		
		for(int j=1; j<BIN_INTERVAL && i<varvalmap->size(); j++) {
		  sum+=(*second_level_sums)[i];
		  firstlevelidx = Bitops.logic_or(firstlevelidx, (*secondlevelvectors)[i++]);
		  preit = curit++;
		}
		curbin.max_val = (*preit).first;
		firstlevelvalue->push_back(curbin);
		
		firstlevelvectors->push_back(firstlevelidx);
		first_level_sums->insert(pair<int, int>(k,sum));//===
		k++;
	}
	clock_t t6 = clock();
	cout<< t6-t5<<endl;
	/*i=0;
	for(auto bitvector:(*first_level_bitmap))
	{
		cout<<"bitvector "<<i<<endl;
		int j=0;
		for(auto chunk : bitvector)
		{
			cout<<"chunk"<<j<<":"<<chunk.size()<<" ";
			j++;
		}
		cout<<endl;
		i++;
	}*/
	itemsCount = items;
	cout<<"Bitmap created"<<endl;
	cout<<"Item Counts:"<<itemsCount<<endl;
	cout<<"number of low-level bins:"<<(*secondlevelvectors).size()<<endl;
	cout<<"number of high-level bins:"<<(*firstlevelvectors).size()<<endl;
	cout<<"======================\n";
	calcPreAgg();
	save_bitmap(dir);	
}
template <class a_type> 
Bitmap<a_type>::Bitmap(string dir){	
	cout<<"Loading bitmap...\n";
	// cout<<dir<<endl;
	load_bitmap(dir);
	cout<<"======================\n";
}
/*
 * The destructor function
 */
template <class a_type>
Bitmap<a_type>::~Bitmap() {
	// TODO Auto-generated destructor stub
	delete firstlevelvalue;
	delete varvalmap;
	delete firstlevelvectors;
	delete secondlevelvectors;
}

template <class a_type> 
/*
 * Set precision(number of bins) of current indexing based on value ranges
 * In current solution, we control the total number of bins between [100, 999].
 * Users can modify this code to generate flexible bin numbers
 * In the future, we plan to provide an interface for users to specify the bin number
 */ 
void Bitmap<a_type>::setPrecision() {
  
  //data type issue
  
  if(SET_PREC)
  {
	  a_type diff = maxvalue - minvalue;
	  if(diff > 1000000)
		this->numpres = 0.0001;
	  else if(diff > 100000)
		this->numpres = 0.001;
	  else if(diff > 10000)
		this->numpres = 0.01;
	  else if(diff > 1000)
		this->numpres = 0.1;
	  else if(diff > 100)
		this->numpres = 1;
	  else if(diff > 1)
		this->numpres = 10;
	  else if(diff > 0.1)
		this->numpres = 100;
	  else if(diff > 0.01)
	   this->numpres = 1000;
	  else
	   this->numpres = 10000;
   }
   else
	this->numpres = 1;
}

template <class a_type>
void Bitmap<a_type>:: calcPreAgg()
{
	int i = 0;
	for(auto vec:*secondlevelvectors )
	{
		stat s;
		boost::dynamic_bitset<> uncompressed_bitvector = Bitops.uncompressIndex(vec,itemsCount);
		s.count = uncompressed_bitvector.count();
		s.sum = (*second_level_sums)[i];
		second_level_statistics.push_back(s);
		i++;
		//cout<<s.count<<"vv	";
		
	}
	i = 0;
	for(auto vec:*firstlevelvectors )
	{
		stat s;
		boost::dynamic_bitset<> uncompressed_bitvector = Bitops.uncompressIndex(vec,itemsCount);
		s.count = uncompressed_bitvector.count();
		s.sum = (*first_level_sums)[i];
		first_level_statistics.push_back(s);
		i++;
		//cout<<s.count<<"gg	";
		
	}
}


////########################### gets ######################################
template <class a_type>
float Bitmap<a_type>:: get_first_level_sum(int binNumber)
{
	//cout<<binNumber<<"  "<<first_level_statistics.size();
	//assert(false);
	return first_level_statistics[binNumber].sum;
	//return (pair<float,float>)((*first_level_statistics)[binNumber].sum,(*first_level_statistics)[binNumber].count);
}
template <class a_type>
float Bitmap<a_type>:: get_first_level_count(int binNumber)
{
	return first_level_statistics[binNumber].count;
	//return (pair<float,float>)((*first_level_statistics)[binNumber].sum,(*first_level_statistics)[binNumber].count);
}
template <class a_type>
float Bitmap<a_type>:: get_second_level_sum(int binNumber)
{
	return second_level_statistics[binNumber].sum;
	//return (pair<float,float>)((*first_level_statistics)[binNumber].sum,(*first_level_statistics)[binNumber].count);
}
template <class a_type>
float Bitmap<a_type>:: get_second_level_count(int binNumber)
{
	return second_level_statistics[binNumber].count;
	//return (pair<float,float>)((*first_level_statistics)[binNumber].sum,(*first_level_statistics)[binNumber].count);
}

template <class a_type>
float  Bitmap<a_type>:: get_numpres()
{
	return numpres;
}
/*
 * returns a pointer to a specific bin in the bitmap*/
template <class a_type>
vector<size_t> Bitmap<a_type>::get_firstlevelvector(int binNumber)
{
	return firstlevelvectors->at(binNumber);
}
template <class a_type>
vector<struct index_bin>*Bitmap<a_type>:: get_firstlevelvalue()
{
	return firstlevelvalue;
}
/* returns the value of the node 
	input: node_number
	output: the bin <min,max> values as a pair<int,int>; returns NULL if the value is not in none of the bins
*/
template <class a_type>
pair<int,int> * Bitmap<a_type>:: get_value(int node_number)
{	
	pair<int,int>* result = NULL; 
	int number_of_bins = (*firstlevelvectors).size();
	
	/* 	//////// naive get_node_value approach
	 * fun to work with
	 * well communicate
	 * know your shet 
	boost::dynamic_bitset<>bit_node_number(itemsCount);
	bit_node_number.set(node_number);
	for(int i = number_of_bins-1;i>=0;i--)
	{
		//cout<<i<<"   "<<(*firstlevelvalue)[i].min_val/numpres<<endl;
		//cout<<node_number<<"  "<<bit_node_number<<endl<<uncompressIndex((*firstlevelvectors)[i], itemsCount)<<endl<<"-------------\n";
		//assert(false);
		if((bit_node_number&uncompressIndex((*firstlevelvectors)[i], itemsCount)).any())
		{
			result = new pair<int,int>;
			result->first = (*firstlevelvalue)[i].min_val/numpres;
			result->second= (*firstlevelvalue)[i].max_val/numpres;
			return result;
		}
	}*/
		
	//////// fast get_node_value approach
	
	////naive bitset compression approach	
	/*boost::dynamic_bitset<>bit_node_number(itemsCount);//TODO: it ideally should be the lenght of a compressed bin after compression. itemsCount or 1, node number +1?
	bit_node_number.set(node_number);
	vector<size_t>comp_node_number = compressBitset(bit_node_number);
	*/

	
		
	////fast setbit compression	
	//clock_t tq = clock();
	 vector<size_t> comp_node_number ;	 
	 int HZero = node_number - node_number%31;
	 if(HZero>0)
	 {
		size_t HZeroW = 0x80000000 + HZero;
		comp_node_number.push_back(HZeroW);
	 }
	 int OneW=0;
	 if(itemsCount<31)
	 	 OneW = 1<<itemsCount-(node_number-HZero)-1;
	 else
	 	 OneW = 1<<31-(node_number-HZero)-1;
	 comp_node_number.push_back(OneW);
	 
	 int LZero = itemsCount-HZero-31;
	 if(LZero>0)
	 {
		size_t LZeroW = 0x80000000 + LZero;
		comp_node_number.push_back(LZeroW);
	 }
	 //gtt += clock()-tq;
	 /*cout<<node_number<<":";
	 for(auto word : comp_node_number_1)
		cout<<hex<<word<<"	";
		//cout<<dec<<word<<"	";
	 cout<<endl<<"=========\n";
	 
	 cout<<"\n++++++++++++++++++\n";
	
	cout<<node_number<<":";
	for(auto word : comp_node_number)
		cout<<hex<<word<<"	";
		//cout<<dec<<word<<"	";
	cout<<endl<<"=========\n";
	*/
	for(int i = number_of_bins-1;i>=0;i--)
	{
		//// =================   debug ==============================
			/*//cout<<dec<<"\n\nn#:"<<node_number<<" idx:"<<itemsCount-node_number<<hex<<"\nCompN#:\n";
			cout<<endl;
			for(auto w: comp_node_number)
				cout<<hex<<w<<"	";
			cout<<endl<<"bin:\n";
			for(auto w: (*firstlevelvectors)[i])
				cout<<w<<"	";
			cout<<endl<<dec<<endl;
			//assert(false);*/
		////=========================================================
		// clock_t tq = clock();

		vector<size_t> comp_and_result = Bitops.logic_and_ref(comp_node_number,(*firstlevelvectors)[i]); // pass by reference Bottle neck
		
		// gtt += clock()-tq;
		//k0++;
		//if(clock()-tq>0)
			
			//cout<<clock()-tq<<"comp_node_number.size:"<<comp_node_number.size()<<"  (*firstlevelvectors)[i].size:"<<(*firstlevelvectors)[i].size()<<endl;
		
		
	
	
		for(size_t word : comp_and_result) //if(compressed_and_result.any())
		{
			int WType = Bitops.word_type(word);
			if(DebugMode)
				cout<<"word type:"<<WType<<"	word:"<<hex<<word<<endl;
			if( WType == 0 || WType == 2)// if the word is literal or ones fill word 
			{
				if(DebugMode)
					cout<<"debug6\n";
				result = new pair<int,int>;
				result->first = (*firstlevelvalue)[i].min_val/numpres;
				result->second= (*firstlevelvalue)[i].max_val/numpres;
				return result;
			}
		}
	
	}
	
	if (result == NULL)
	{
			cout<<"Exception: debug8-null val returned\n";
			cout<<node_number<<endl;
	}
	return result; 
}

/*
 * Find the cardinality of high-level bitmap indices (Omid: or number of high-level bins)
 */
template <class a_type> 
unsigned long Bitmap<a_type>::getL1Size() {
  return firstlevelvectors->size();
}

/*
 * Find the cardinality of low-level bitmap indices(Omid: or number of low-level bins)
 */
template <class a_type>
unsigned long Bitmap<a_type>::getL2Size() {
  return secondlevelvectors->size();
}

template <class a_type>
int Bitmap<a_type>::get_max()
{return maxvalue;}

template <class a_type>
int Bitmap<a_type>::get_min()
{return minvalue;}

template <class a_type>
long Bitmap<a_type>::get_count()
{return itemsCount;}
////########################### prints ###################################
template <class a_type>
void Bitmap<a_type>::print_min_max()
{
	cout<<"========================"<<endl;
	int i=0;
	for(auto elem: *firstlevelvalue)
	{
		cout<<"bin#:"<<i<<" min: "<<elem.min_val/numpres<<"	max:"<<elem.max_val/numpres<<endl;
		i++;
	}
	cout<<endl;
}

template <class a_type>
void Bitmap<a_type>:: print_first_level_uncompressed_vectors()
{
	int i=0;
	for(auto vec:*firstlevelvectors )
	{
		for(auto w: vec)
		{
			boost::dynamic_bitset<> x (32,w);
			cout<<x<<" ";
		}
		cout<<dec<<"<=== b"<<i<<endl;
		i++;
	}		
}

template <class a_type>
void Bitmap<a_type>:: print_varvalmap()
{
	for(auto elem: *varvalmap)
		cout<<"key:"<<elem.first<<", val: "<<elem.second<<endl;
}
	
template <class a_type>
void Bitmap<a_type>:: print_second_level_uncompressed_vectors()
{
	int i=0;
	//cout<<(*secondlevelvectors).size()<<endl;
	for(auto vec:*secondlevelvectors )
	{
		for(auto w: vec)
		{
			boost::dynamic_bitset<> x (32,w);
			cout<<x<<" ";
		}
		cout<<dec<<"<=== b"<<i<<endl;
		i++;
	}
	
}

template <class a_type>
void Bitmap<a_type>:: print_stat()
{
	
	cout<<"===========second level stats============="<<endl;
	int i = 0;
	for(auto s:second_level_statistics)
	{
		cout<<"bin#: "<<i<<" count: "<<s.count<<" sum: "<<s.sum<<endl;
		i++;
	}
	cout<<"===========first level stats============="<<endl;
	i = 0;
	//cout<<"common!"<<first_level_statistics[0].min;
	for(auto s:first_level_statistics)
	{
		cout<<"bin#: "<<i<<" count: "<<s.count<<" sum: "<<s.sum<<endl;
		i++;
	}
}
//// ########################################## save/load fucntions ##########################################
template <class a_type>
void Bitmap<a_type>::save_bitmap(string dir)
{
	save_variables(dir+"variables.txt");
	save_secondlevelvectors(dir+"secondlevelvectors.txt");
	save_firstlevelvectors(dir+"firstlevelvectors.txt");

	save_firstlevelvalue(dir+"firstlevelvalue.txt");

	save_second_level_statistics(dir+"second_level_statistics.txt");
	save_first_level_statistics(dir+"first_level_statistics.txt");
	
	// save_second_level_sums(dir);
	// save_first_level_sums(dir);
}
template <class a_type>
void Bitmap<a_type>::save_variables(string dir)
{
	////file structure:
	////first line: numpress minvalue maxvalue itemCounts
	cout<<"Saving variables on"<<dir<<endl;
	ofstream myfile;
	myfile.open (dir);
	if(myfile.is_open())
		myfile<<numpres<<" "<<minvalue<<" "<<maxvalue<<" "<<itemsCount<<endl;
	else
		cout<<"Saving secondlevelvectors on"<<dir<<endl;		
	myfile.close();
}

template <class a_type>
void Bitmap<a_type>::save_secondlevelvectors(string dir)
{
	////file structure:
	////first line: number of bins
	////other lines: start with number or words in the bin, followed by other words
	cout<<"Saving secondlevelvectors on"<<dir<<endl;

	ofstream myfile;
	myfile.open (dir);
	if(myfile.is_open())
	{
		myfile<<secondlevelvectors->size()<<endl;          ////first line: number of bins
		for(auto bin :(*secondlevelvectors))
		{
			myfile<<bin.size()<<" ";
			for(size_t word : bin)
			{
				myfile<<word<<" ";
			}
			myfile<<endl;
		}
	}
	else
		cout<<"Could not open file "<<dir<<endl;												
	myfile.close();
}
template <class a_type>
void Bitmap<a_type>::save_firstlevelvectors(string dir)
{
	////file structure:
	////first line: number of bins
	////other lines: start with number or words in the bin, followed by other words
	cout<<"Saving firstlevelvectors on"<<dir<<endl;

	ofstream myfile;
	myfile.open (dir);
	if(myfile.is_open())
	{	
		myfile<<firstlevelvectors->size()<<endl;          ////first line: number of bins
		for(auto bin :(*firstlevelvectors))
		{
			myfile<<bin.size()<<" ";
			for(size_t word : bin)
			{
				myfile<<word<<" ";
			}
			myfile<<endl;
		}
	}
	else
		cout<<"Could not open file "<<dir<<endl;										
	myfile.close();
}
template <class a_type>
void Bitmap<a_type>::save_firstlevelvalue(string dir)
{
	////file structure:
	////first line: number of lines
	////other lines: min max
	cout<<"Saving firstlevelvalue on"<<dir<<endl;
	
	ofstream myfile;
	myfile.open (dir);
	if(myfile.is_open())
	{	
		myfile<<firstlevelvalue->size()<<endl;
		for(auto bin_boundary :(*firstlevelvalue))
		{
			myfile<<bin_boundary.min_val<<" "<<bin_boundary.max_val<<endl;
		}
	}
	else
		cout<<"Could not open file "<<dir<<endl;								
	myfile.close();
}

template <class a_type>
void Bitmap<a_type>::save_second_level_statistics(string dir)
{
	////file structure:
	////first line: number of lines
	////other lines: sum count min max
	cout<<"Saving second_level_statistics on"<<dir<<endl;
	
	ofstream myfile;
	myfile.open (dir);
	if(myfile.is_open())
	{
		myfile<<second_level_statistics.size()<<endl;          ////first line: number of bins
		for(auto bin_stat :second_level_statistics)
		{
			myfile<<bin_stat.sum<<" "<<bin_stat.count<<" "<<bin_stat.min<<" "<<bin_stat.max<<endl;
		}
	}
	else
		cout<<"Could not open file "<<dir<<endl;						
	myfile.close();
}
template <class a_type>
void Bitmap<a_type>::save_first_level_statistics(string dir)
{
	////file structure:
	////first line: number of lines
	////other lines: sum count min max
	cout<<"Saving first_level_statistics on"<<dir<<endl;
	
	ofstream myfile;
	myfile.open (dir);
	if(myfile.is_open())
	{
		myfile<<first_level_statistics.size()<<endl;          ////first line: number of bins	
		for(auto bin_stat :first_level_statistics)
		{
			myfile<<bin_stat.sum<<" "<<bin_stat.count<<" "<<bin_stat.min<<" "<<bin_stat.max<<endl;
		}
	}
	else
		cout<<"Could not open file "<<dir<<endl;				
	myfile.close();
}

template <class a_type>
void Bitmap<a_type>:: load_bitmap(string dir)
{
	load_variables(dir+"variables.txt");
	load_secondlevelvectors(dir+"secondlevelvectors.txt");
	load_firstlevelvectors(dir+"firstlevelvectors.txt");

	load_firstlevelvalue(dir+"firstlevelvalue.txt");

	load_second_level_statistics(dir+"second_level_statistics.txt");
	load_first_level_statistics(dir+"first_level_statistics.txt");;
	
// 	load_second_level_sums(dir);
// 	load_first_level_sums(dir);
} 
template <class a_type>
void Bitmap<a_type>::load_variables(string dir)
{
	////file structure:
	////first line: numpress minvalue maxvalue itemCounts
	cout<<"Loading variables from "<<dir<<endl;
	// cout<<dir<<endl;
	ifstream myfile;
	myfile.open (dir);
	if(myfile.is_open())
	{
		myfile>>numpres>>minvalue>>maxvalue>>itemsCount;
		// cout<<"here"<<itemsCount<<endl;
	}
	else{
		cout<<"Could not open file "<<dir<<endl;		
	}
	myfile.close();
}

template <class a_type>
void Bitmap<a_type>::load_secondlevelvectors(string dir)
{
	////file structure:
	////first line: number of bins
	////other lines: start with number or words in the bin, followed by other words
	cout<<"Loading secondlevelvectors from "<<dir<<endl;

	ifstream myfile;
	myfile.open (dir);
	if(myfile.is_open())
	{	
		int number_of_bins = 0;
		myfile>>number_of_bins;          ////first line: number of bins
		secondlevelvectors = new vector<vector<size_t>>;
		for(int i = 0;i<number_of_bins;i++)
		{
			int number_of_words = 0;
			vector<size_t> bin;
			myfile>>number_of_words;
			for(int j = 0;j< number_of_words;j++)
			{
				size_t word = 0;
				myfile>>word;
				bin.push_back(word);
			}
			secondlevelvectors->push_back(bin);
		}
	}
	else{
		cout<<"Could not open file "<<dir<<endl;		
	}
	myfile.close();
}
template <class a_type>
void Bitmap<a_type>::load_firstlevelvectors(string dir)
{
	////file structure:
	////first line: number of bins
	////other lines: start with number or words in the bin, followed by other words
	cout<<"Loading firstlevelvectors from "<<dir<<endl;

	ifstream myfile;
	myfile.open (dir);
	if(myfile.is_open())
	{
		int number_of_bins = 0;
		myfile>>number_of_bins;          ////first line: number of bins
		firstlevelvectors = new vector<vector<size_t>>;
		for(int i = 0;i<number_of_bins;i++)
		{
			int number_of_words = 0;
			vector<size_t> bin;
			myfile>>number_of_words;
			for(int j = 0;j< number_of_words;j++)
			{
				size_t word = 0;
				myfile>>word;
				bin.push_back(word);
			}
			firstlevelvectors->push_back(bin);
		}
	}
	else
	{
		cout<<"Could not open file "<<dir<<endl;				
	}
	myfile.close();
}
template <class a_type>
void Bitmap<a_type>::load_firstlevelvalue(string dir)
{
	////file structure:
	////first line: number of lines
	////other lines: min max
	cout<<"Loading firstlevelvalue from "<<dir<<endl;
	
	ifstream myfile;
	myfile.open (dir);
	if(myfile.is_open())
	{	
		firstlevelvalue = new vector<struct index_bin>;
		int number_of_bins = 0;
		myfile>>number_of_bins;////first line: number of lines
		for(int i = 0;i<number_of_bins;i++)
		{
			index_bin t;
			myfile>>t.min_val>>t.max_val;
			firstlevelvalue->push_back(t);
		}
	}
	else{
		cout<<"Could not open file "<<dir<<endl;		
	}
	myfile.close();
}
template <class a_type>
void Bitmap<a_type>::load_second_level_statistics(string dir)
{
	////file structure:
	////first line: number of lines
	////other lines: sum count min max
	cout<<"Loading second_level_statistics from "<<dir<<endl;
	
	ifstream myfile;
	myfile.open (dir);	
	if(myfile.is_open())
	{
		int number_of_bins = 0;
		myfile>>number_of_bins;////first line: number of lines
		for(int i = 0;i<number_of_bins;i++)
		{
			stat t;
			myfile>>t.sum>>t.count>>t.min>>t.max;
			second_level_statistics.push_back(t);
		}
	}
	else
		cout<<"Could not open file "<<dir<<endl;				
	myfile.close();
}
template <class a_type>
void Bitmap<a_type>::load_first_level_statistics(string dir)
{
	////file structure:
	////first line: number of lines
	////other lines: sum count min max
	cout<<"Loading first_level_statistics from "<<dir<<endl;
	
	ifstream myfile;
	myfile.open (dir);
	if(myfile.is_open())
	{	
		int number_of_bins = 0;
		myfile>>number_of_bins;////first line: number of lines
		for(int i = 0;i<number_of_bins;i++)
		{
			stat t;
			myfile>>t.sum>>t.count>>t.min>>t.max;
			first_level_statistics.push_back(t);
		}
	}
	else
		cout<<"Could not open file "<<dir<<endl;						
	myfile.close();
}
template class Bitmap<double>;
template class Bitmap<float>;
template class Bitmap<int>;
template class Bitmap<long>;
template class Bitmap<size_t>;
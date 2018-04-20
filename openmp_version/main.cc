#include<iostream>
#include "mybitops.h"
#include <fstream>
#include <cstdlib>

using namespace std;
#define DATA_GENERATION false
void save_vector(vector<size_t>& input_vector ,string dir, int repeat_factor)
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
        save_vector(comp_bitset1,"./vec4", 1000);
    }
    else
    {
        comp_bitset1 = load_vector("../data/lvec1");
        comp_bitset2 = load_vector("../data/lvec1");        
    }

    // clock_t t2 = clock();
    Bitops.parallel_and(comp_bitset1,comp_bitset2,NUM_THREADS);
    // clock_t t3 = clock();
    // cout<<"parallel_and time:"<< t3-t2<<endl;

    clock_t t4 = clock();
    Bitops.logic_and_ref(&comp_bitset1,&comp_bitset2,0,comp_bitset1.size()-1,0,comp_bitset2.size()-1);    
    clock_t t5 = clock();
    cout<<"seq_and time:"<< t5-t4<<endl;
    pthread_exit(NULL);
    return 0;
}
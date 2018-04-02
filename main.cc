#include<iostream>
#include "mybitops.h"
using namespace std;
int main()
{
    size_t items = 10000000;
    float fill_percent = .001;
    boost::dynamic_bitset<> tempvector(items);
    for(int i=0; i<items*fill_percent; i++) {
        int rnd = rand()%items;
        while(tempvector[rnd]==1)
            rnd = rand()%items;
        tempvector[rnd] = 1;
    }
    // cout<<tempvector<<endl;
    mybitops Bitops;
    clock_t t0 = clock();
    vector<size_t> comp_bitset = Bitops.compressBitset(tempvector);
    clock_t t1 = clock();
    cout<<"compression time:"<< t1-t0<<endl;

    clock_t t2 = clock();
    int par_size = (Bitops.parallel_and(comp_bitset,comp_bitset)).size();
    clock_t t3 = clock();
    cout<<"parallel_and time:"<< t3-t2<<endl;
    cout<<"par_and size:"<< par_size<<endl;

    clock_t t4 = clock();
    int seq_size = (Bitops.logic_and(comp_bitset,comp_bitset)).size();    
    clock_t t5 = clock();
    cout<<"seq_and time:"<< t5-t4<<endl;
    cout<<"seq_and size:"<< seq_size<<endl;
    return 0;
}
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
    Bitops.parallel_and();
    return 0;
}
/* 
 * File:   main.cc
 * Author: omid
 * Created on October 18, 2016, 4:11 PM
 * 
 */
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <unordered_map>
#include <limits>
#include <time.h>
#include <chrono>
#include <pthread.h>
#include <omp.h>
#include <math.h>
#include <utility>
using namespace std;
int NUM_THREADS;
struct box 
{
int box_id;
int x;
int y;
int height;
int width;
int num_top_neighbors;
vector<int> top_neighbor_ids;
int num_bottom_neighbors;
vector<int> bottom_neighbor_ids;
int num_left_neighbors;
vector<int> left_neighbor_ids;
int num_right_neighbors;
vector<int> right_neighbor_ids;
double box_dsv;
double next_box_dsv;
//constructor function
box(){
    /*do nothing*/
}
//set functions======================================
// sets the box ID
void set_id(int id){
    box_id = id;
}
// sets the number of the top neighbors
void set_top_num(int tnum) {
    if(tnum>=0)// make sure input is positive
        num_top_neighbors = tnum;
    else
    {
        cout<<"bad input for top neighbor number:"<<tnum;
        exit(10);
    }
}
// sets the number of the bottom neighbors
void set_bot_num(int bnum) {        
    if(bnum>=0)// make sure input is positive
        num_bottom_neighbors = bnum;
    else
    {
        cout<<"bad input for bottom neighbor number:"<<bnum;
        exit(10);
    }
}
// sets the number of the left neighbors
void set_left_num(int lnum) {
    if(lnum>=0)// make sure input is positive
        num_left_neighbors = lnum;
    else
    {
        cout<<"bad input for left neighbor number:"<<lnum;
        exit(10);
    }

}
// sets the number of the right neighbors
void set_right_num(int rnum) {
    if(rnum>=0)// make sure input is positive
        num_right_neighbors = rnum;
    else
    {
        cout<<"bad input for right neighbor number:"<<rnum;
        exit(10);
    }

}
// sets the coordinates of each box
void set_position_size(int y, int x, int h, int w) {
    if(x<0 || y<0 || h<0 || w<0)
    {
        cout<<"bad input for the box coordinate: (y:"<<y<<", x:"<<x<<", h:"<<h<<", w:"<<w<<")";
        exit(10);
    }
    this->y = y;
    this->x = x;
    height = h;
    width = w;
}
//sets the IDs of the top neighbors in a vector
void set_top_neighbors(vector<int> TIDs) {
    top_neighbor_ids = TIDs;
}
//sets the IDs of the bottom neighbors in a vector
void set_bot_neighbors(vector<int> BIDs) {
    bottom_neighbor_ids = BIDs;
}
//sets the IDs of the left neighbors in a vector
void set_left_neighbors(vector<int> LIDs) {
    left_neighbor_ids = LIDs;
}
//sets the IDs of the right neighbors in a vector
void set_right_neighbors(vector<int> RIDs) {
    right_neighbor_ids = RIDs;
}
// sets the box temperature
void set_box_dsv(double dsv) {
    box_dsv = dsv;
}
// sets the box next calculated temperature 
void set_box_next_dsv(double Ndsv) 
{
    next_box_dsv = Ndsv;
}
//get functions ======================================
// prints the box
void print_box() {
    cout << "ID:" << box_id << "\n" << "[y:" << y << ",x:" << x << "]" << "[h:" << height << ",w:" << width << "]" << endl;
    cout << "top neighbors number:" << num_top_neighbors << endl;
    cout << "bottom neighbors number:" << num_bottom_neighbors << endl;
    cout << "left neighbors number:" << num_left_neighbors << endl;
    cout << "right neighbors number:" << num_right_neighbors << endl;
    cout << "top neighbors:";
    for (auto it = top_neighbor_ids.begin(); it != top_neighbor_ids.end(); ++it)
        cout << *it << "  ";
    cout << "\nbottom neighbors:";
    for (auto it = bottom_neighbor_ids.begin(); it != bottom_neighbor_ids.end(); ++it)
        cout << *it<< "  ";
    cout << "\nleft neighbors:";
    for (auto it = left_neighbor_ids.begin(); it != left_neighbor_ids.end(); ++it)
        cout << *it << "  ";
    cout << "\nright neighbors:";
    for (auto it = right_neighbor_ids.begin(); it != right_neighbor_ids.end(); ++it )
        cout << *it << "  ";
    cout << "DSV:" << box_dsv;
    cout << "\n=================\n";

}
// returns the x coordinate of the box
int get_x() {
    return x;
}
// returns the y coordinate of the box
int get_y() {
    return y;
}
// returns the perimeter of the box
int get_perimeter() {
    return (2 * (width + height));
}
// return the vector of the IDs of the top neighbors
vector<int>& get_topbN() {
    return top_neighbor_ids;
}
// return the vector of the IDs of the bottom neighbors
vector<int>& get_botN() {
    return bottom_neighbor_ids;
}
// return the vector of the IDs of the left neighbors
vector<int>& get_leftN() {
    return left_neighbor_ids;
}
// return the vector of the IDs of the right neighbors
vector<int>& get_rightN() {
    return right_neighbor_ids;
}
// returns the width of the box
int get_width() {
    return width;
}
// returns the height of the box
int get_height() {
    return height;
}
// returns the temperature of the box
double get_box_dsv() {
    return box_dsv;
}
// returns the next temperature of the box
double get_box_next_dsv() {
            return next_box_dsv;
}
// returns the number of the top neighbors
int get_top_num() {
    return num_top_neighbors;
}
// returns the number of the bottom neighbors
int get_bot_num() {
    return num_bottom_neighbors;
}
// returns the number of the left neighbors
int get_left_num() {
    return num_left_neighbors;
}
// returns the number of the right neighbors
int get_right_num() {
        return num_right_neighbors;
    }
};
//================================The grid elements========================
int number_of_boxes = 0;
int number_of_rows = 0;
int number_of_cols = 0;
double min_dsv = 1000000; 
double max_dsv =-1000000; 
double AFFECT_RATE = 0.1; // the grid temperature dissipation affect rate. default value = 0.1
unordered_map<int, box> grid; //an unordered map grid, ID as key, box as value 
// reads the input file and generates the unordered grid
void read_grid_file() {
    //read first line
    cin >> number_of_boxes >> number_of_rows>>number_of_cols;
    string line;
    int bid = 0;
    while (bid != -1) // generate the grid
    {
        box b;
        //get 2nd line, get id
        cin>>bid;
        if (bid != -1) {
            b.set_id(bid);
            //get line3, get position, h,w
            int y, x, h, w;
            cin >> y >> x >> h>>w;
            b.set_position_size(y,x, h, w);

            //get line4, get top neighbor IDs
            int numTN;
            vector<int> TN_IDS;
            cin>>numTN;
            b.set_top_num(numTN);
            for (int i = 0; i < numTN; i++) {
                int ID;
                cin>> ID;
                TN_IDS.push_back(ID);
            }
            b.set_top_neighbors(TN_IDS);


            //get line5, get bottom numbers
            int numBN;
            vector<int> BN_IDS;
            cin>>numBN;
            b.set_bot_num(numBN);
            for (int i = 0; i < numBN; i++) {
                int ID;
                cin>> ID;
                BN_IDS.push_back(ID);
            }
            b.set_bot_neighbors(BN_IDS);


            //get line6, get left neighbors
            int numLN;
            vector<int> LN_IDS;
            cin>>numLN;
            b.set_left_num(numLN);
            for (int i = 0; i < numLN; i++) {
                int ID;
                cin>> ID;
                LN_IDS.push_back(ID);
            }
            b.set_left_neighbors(LN_IDS);

            //get line6, get right neighbors
            int numRN;
            vector<int> RN_IDS;
            cin>>numRN;
            b.set_right_num(numRN);
            for (int i = 0; i < numRN; i++) {
                int ID;
                cin>> ID;
                RN_IDS.push_back(ID);
            }
            b.set_right_neighbors(RN_IDS);

            //get line8, get DSV
            double dsv;
            cin>>dsv;
            b.set_box_dsv(dsv);
            //add the box to grid
            grid[bid] = b;

        }

    }
}
// commits the grid temperature updates.
void commite_updates() 
{
	double min = 100000;
	double max = -100000;
    for (auto it = grid.begin(); it != grid.end(); ++it) //for each box in grid
    {
        double next_dsv = it->second.get_box_next_dsv();
        it->second.set_box_dsv(next_dsv); //b.set_box_dsv(b.get_box_next_dsv()
        if (next_dsv < min) //update minimum dsv
        {
            min = next_dsv;
        }
        if (next_dsv > max) //update maximum dsv
        {
            max = next_dsv;
        }
   }
    max_dsv = max;
    min_dsv = min;
}
// the convergence condition
bool converged(double epsilon) {
    return ((max_dsv - min_dsv) < epsilon);
}
// updates the temperature of the box that its id is boxID.
void *calc_updates(void *arguments) {
    long lower = ((pair<long,long> *)arguments)->first;
    long higher = ((pair<long,long> *)arguments)->second;
    //cout<<"L: "<<lower<<" H: "<<higher<<endl;
    for(long boxID = lower;boxID<= higher;boxID++)
    {
	    int cur_TL_Y = grid[boxID].get_y(); //top left y coordinates of the current box
	    int cur_TL_X = grid[boxID].get_x(); //top left x coordinate of the current box
	    int cur_h = grid[boxID].get_height();
	    int cur_w = grid[boxID].get_width();
	    vector<int> TopIDs = grid[boxID].get_topbN();
	    vector<int> BotIDs = grid[boxID].get_botN();
	    vector<int> LeftIDs = grid[boxID].get_leftN();
	    vector<int> RightIDs = grid[boxID].get_rightN();
	    int cur_BR_X = cur_TL_X + cur_w; //bottom right x coordinate of the current box
	    int cur_BR_Y = cur_TL_Y + cur_h; //bottom right y coordinates of the current box
	    double current_temp = grid[boxID].get_box_dsv();
	    //begin calculate average adjacent temperature ===================================
	    double perimeter = grid[boxID].get_perimeter(); //get current box perimeter
	    double sum_temp = 0;
	    if (grid[boxID].get_top_num() == 0)// if zero top boxes
		sum_temp +=0;// current_temp; //consider same temperature as the current box
	    else 
	    {

		for (auto it = TopIDs.begin(); it != TopIDs.end(); ++it)// for each top neighbor box B
		{
		    int ID = *it;
		    double neigh_dsv = grid[ID].get_box_dsv();// get the temperature of the neighbor box B
		    int neigh_TL_X = grid[ID].get_x(); // get top left x of the neighbor box B
		    int neigh_w = grid[ID].get_width();// get the width of the neighbor box B
		    int neigh_BR_X = neigh_TL_X+neigh_w;// calculate the bottom right x of the neighbor box B
		    int contact_surface = 0;// keeps the contact surface of the neighbor box B
		    if(neigh_TL_X <= cur_TL_X and neigh_BR_X<=cur_BR_X) // calculate the contact surface
		        contact_surface = neigh_BR_X - cur_TL_X;
		    else if(neigh_TL_X <= cur_TL_X and neigh_BR_X>cur_BR_X)
		        contact_surface = cur_BR_X - cur_TL_X;
		    else if(neigh_TL_X > cur_TL_X and neigh_BR_X<=cur_BR_X)
		        contact_surface = neigh_BR_X - neigh_TL_X;
		    else if(neigh_TL_X > cur_TL_X and neigh_BR_X>cur_BR_X)
		        contact_surface = cur_BR_X- neigh_TL_X;
		    else
		        cout<<"something bad happened in top code!\n";
		    sum_temp += (neigh_dsv * contact_surface); //sum += neighbor's temperature *contact surface of the neighbor with current box
	       }
	    }

	    if (grid[boxID].get_bot_num() == 0)// if zero button boxes
		sum_temp +=0;//current_temp; //consider same temperature as the current box
	    else 
	    {

		for (auto it = BotIDs.begin(); it != BotIDs.end(); ++it)// for each bottom neighbor box B
		{
		    int ID = *it;
		    double neigh_dsv = grid[ID].get_box_dsv();// get the temperature of the neighbor box B
		    int neigh_TL_X = grid[ID].get_x();// get top left x of the neighbor box B
		    int neigh_w = grid[ID].get_width();// get width of the neighbor box B
		    int neigh_BR_X = neigh_TL_X+neigh_w;// calculate the bottom right x of the neighbor box B
		    int contact_surface = 0;// keeps the contact surface of the neighbor box B
		    if(neigh_TL_X <= cur_TL_X and neigh_BR_X<=cur_BR_X)// calculate the contact surface
		        contact_surface = neigh_BR_X - cur_TL_X;
		    else if(neigh_TL_X <= cur_TL_X and neigh_BR_X>cur_BR_X)
		        contact_surface = cur_BR_X - cur_TL_X;
		    else if(neigh_TL_X > cur_TL_X and neigh_BR_X<=cur_BR_X)
		        contact_surface = neigh_BR_X - neigh_TL_X;
		    else if(neigh_TL_X > cur_TL_X and neigh_BR_X>cur_BR_X)
		        contact_surface = cur_BR_X- neigh_TL_X;
		    else
		        cout<<"something bad happened in bottom code!\n";
		    sum_temp += (neigh_dsv * contact_surface); //sum += neighbor's temperature *contact surface of the neighbor with current box

		}

	    }
	    if (grid[boxID].get_left_num() == 0)// if zero left boxes
		sum_temp +=0;// current_temp; //consider same temperature as the current box
	    else 
	    {
		for (auto it = LeftIDs.begin(); it != LeftIDs.end(); ++it)// for each left neighbor box B
		{
		    int ID = *it;
		    double neigh_dsv = grid[ID].get_box_dsv();// get the temperature of the neighbor box B
		    int neigh_TL_Y = grid[ID].get_y();// get top left y of the neighbor box B
		    int neigh_h = grid[ID].get_height();// get height of the neighbor box B
		    int neigh_BR_Y = neigh_TL_Y+neigh_h;// calculate bottom right y of the neighbor box B
		    int contact_surface = 0;// keeps the contact surface of the neighbor box B
		    if(neigh_TL_Y <= cur_TL_Y and neigh_BR_Y<=cur_BR_Y)// calculate the contact surface
		        contact_surface = neigh_BR_Y - cur_TL_Y;
		    else if(neigh_TL_Y <= cur_TL_Y and neigh_BR_Y>cur_BR_Y)
		        contact_surface = cur_BR_Y - cur_TL_Y;
		    else if(neigh_TL_Y > cur_TL_Y and neigh_BR_Y<=cur_BR_Y)
		        contact_surface = neigh_BR_Y - neigh_TL_Y;
		    else if(neigh_TL_Y > cur_TL_Y and neigh_BR_Y>cur_BR_Y)
		        contact_surface = cur_BR_Y- neigh_TL_Y;
		    else
		        cout<<"something bad happened in left code!\n";
		    sum_temp += (neigh_dsv * contact_surface); //sum += neighbor's dsv *contact surface of the neighbor with current box

		}
	    }

	    if (grid[boxID].get_right_num() == 0)// if zero right boxes
		sum_temp +=0;// current_temp; //consider same temperature as the current box
	    else
	    {
		for (auto it = RightIDs.begin(); it != RightIDs.end(); ++it)// for each left neighbor box B
		{
		    int ID = *it;
		    double neigh_dsv = grid[ID].get_box_dsv();// get the temperature of the neighbor box B
		    int neigh_TL_Y = grid[ID].get_y();// get top left y of the neighbor box B
		    int neigh_h = grid[ID].get_height();// get height of the neighbor box B
		    int neigh_BR_Y = neigh_TL_Y+neigh_h;// calculate bottom right y of the neighbor box B
		    int contact_surface = 0;// keeps the contact surface of the neighbor box B
		    if(neigh_TL_Y <= cur_TL_Y and neigh_BR_Y<=cur_BR_Y)
		        contact_surface = neigh_BR_Y - cur_TL_Y;
		    else if(neigh_TL_Y <= cur_TL_Y and neigh_BR_Y>cur_BR_Y)
		        contact_surface = cur_BR_Y - cur_TL_Y;
		    else if(neigh_TL_Y > cur_TL_Y and neigh_BR_Y<=cur_BR_Y)
		        contact_surface = neigh_BR_Y - neigh_TL_Y;
		    else if(neigh_TL_Y > cur_TL_Y and neigh_BR_Y>cur_BR_Y)
		        contact_surface = cur_BR_Y- neigh_TL_Y;
		    else
		        cout<<"something bad happened in right code!\n";
		    sum_temp += (neigh_dsv * contact_surface); //sum += neighbor's dsv *contact surface of the neighbor with current box

		}
	    }

	    double avg_adj_temp = sum_temp / perimeter;
	    //end calculate average adjacent temperature=======================================
	    /***************** begin temperature migration****************************************/
	    double new_dsv = current_temp + (avg_adj_temp - current_temp) * AFFECT_RATE;// compute new DSV for the current box
	    grid[boxID].set_box_next_dsv(new_dsv);
      }
    //pthread_exit(NULL);
}
// Dissipation phase ************************
void dissipate(double epsilon) 
{
    long requested_thread_number;
    if(number_of_boxes>=NUM_THREADS) // at maximum request only one thread per box
        requested_thread_number = NUM_THREADS;        
    else
        requested_thread_number = number_of_boxes;
    // Request threads
    omp_set_num_threads(requested_thread_number);
    cout<< "requested number of threads is:"<<requested_thread_number<<endl;
    //timing stuff
    int iterations = 0;
    cout<<"Start!; it may take up to 4 minutes..."<<endl;
    auto t_chrono = chrono::system_clock::now();
    time_t timer1, timer2;
    time(&timer1);
    clock_t t;
    t = clock();
    //=============
    long actual_num_threads; // it is going to keep the actual number of threads assigned by omp
    do // this is the "repeat until converged" loop
    {
        // parallelize calls to calc_updates()
        #pragma  omp parallel //num_threads(10)
        {
            int id, nthrds; 
            id = omp_get_thread_num();// query the current thread id
            //cout<<"hello from: "<<id<<endl;        
            nthrds = omp_get_num_threads();//query thread number
            if(id == 0) actual_num_threads = nthrds; // good practice: let thread 0 do this
            //############ load distribution ##################
            // I used "block data distribution" to avoid false sharing
            int factor;// keeps the factor of box per thread
            if(number_of_boxes>=nthrds) 
                factor= floor(number_of_boxes/nthrds);
            else
                factor= 1;
            pair<long,long> * arguments;
            arguments = new pair<long,long>;
            if(id != nthrds-1)// if it is not the last thread
            { //assign the appropriate block of data to the thread
                arguments->first = id*factor;// the start point of the block associated the id-th thread
                arguments->second = (id+1)*factor-1;// the end point of the block associated the id-th thread 
            }
            else // if it is the last thread
            { // assign all the remaining boxes to it(the last thread).
                arguments->first = id*factor;// the start point of the block associated the last thread
                arguments->second = number_of_boxes -1;// the end point of the block associated the last thread
            }
            //####### call the thread safe function "calc_updates" ############
            calc_updates(arguments);
        } // joins :)
        commite_updates();
        iterations++;
        } while (!converged(epsilon));// && iterations<5);
        t = clock() - t;
        time(&timer2);
        double seconds = timer2-timer1;
        auto t_chronod = chrono::system_clock::now() - t_chrono;
        cout<< "Actual number of threads is:"<<requested_thread_number<<endl;
        cout<<"dissipation converged in "<< iterations<<" iterations,\n"; 
        cout<<"\t with max DSV = "<<max_dsv<<"\t and min DSV = "<<min_dsv<<endl;
        cout<<"\t affect rate = "<<AFFECT_RATE<<";\t\tepsilon = "<<epsilon<<endl<<endl;
        cout<<"elapsed convergence loop time (clock):"<<t<<endl;
        cout<<"elapsed convergence loop time (time):"<<seconds<<endl;
        cout<<"elapsed convergence loop time (chrono):"<< chrono::duration<double,std::milli>(t_chronod).count()<<endl;
        cout<<"***********************************************************************"<<endl;
}
int main(int argc, char** argv) 
{
    cout<<"Usage: ./disposal affect_rate epsilon num_threads < input_test_file\n";
    cout<<"***************************************************************\n";
    AFFECT_RATE = atof(argv[1]);
    double Epsilon=atof(argv[2]);
    NUM_THREADS =atoi(argv[3]);
    cout<<"start reading file...\n";
    read_grid_file();
    dissipate(Epsilon);
    // pthread_exit(NULL);
    return 0;
}

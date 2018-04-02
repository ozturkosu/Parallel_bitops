/* 
 * File:   main.cc
 * Author: omid
 * Created on October 6, 2016, 1:47 PM
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
#include <math.h>
#include <utility>
using namespace std;
double Epsilon;
int NUM_THREADS;
static pthread_barrier_t barrier1;
static pthread_barrier_t barrier2;
struct th_arg{
pair<long,long> * arguments;
long  tid;
};
struct box {
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
	//cout<<"next dsv:"<<next_dsv<<" min_dsv:"<<min_dsv<<endl;
        if (next_dsv < min) //update minimum dsv
        {
            min = next_dsv;
            //cout<<"min updated\n";
        }
	//cout<<"next dsv:"<<next_dsv<<" max_dsv:"<<max_dsv<<endl;
        if (next_dsv > max) //update maximum dsv
        {
            max = next_dsv;
           // cout<<"max updated\n";
        }
   }
    max_dsv = max;
    min_dsv = min;
    //cout<<"min:"<<min_dsv<<" max:"<<max_dsv<<endl;
}
// the convergence condition
bool converged(double epsilon) {
    return ((max_dsv - min_dsv) < epsilon);
}
// updates the temperature of the box that its id is boxID.
void *calc_updates(void *struc) {
	th_arg *thread_arg = (th_arg *)struc;
	long TID = thread_arg->tid;
	long lower = thread_arg->arguments->first;
	long higher = thread_arg->arguments->second;
//cout<<lower<<" "<<higher<<" "<<TID<<endl;
    //(pair<long,long> *)
    //long lower = ((pair<long,long> *)arguments)->first;
    //long higher = ((pair<long,long> *)arguments)->second;
    //cout<<"L: "<<lower<<" H: "<<higher<<endl;
 do{
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
	//    cout<<boxID<<" old:"<<current_temp<<" new:"<<new_dsv<<endl;
      }//cout <<"here4"<<endl;
     pthread_barrier_wait(&barrier1);//cout <<"here2"<<endl;
     if(TID == 0){
	commite_updates();
	//cout <<"here"<<endl;
	}
     pthread_barrier_wait(&barrier2);
     }while (!converged(Epsilon));// && iterations<5);
    //pthread_exit(NULL);
}
// Dissipation phase ************************
void dissipate() 
{
    int factor;
    if(number_of_boxes>=NUM_THREADS) 
        factor= floor(number_of_boxes/NUM_THREADS);
    else
        factor= 1;
    // multi threading====================
    pthread_t *threads;
    threads = new pthread_t[NUM_THREADS];
    void *th_status;
    int rc;
    //======================================
    int iterations = 0;

/////////////////////////////////

	long active_thread_number;
	if(number_of_boxes>=NUM_THREADS) // at maximum create only one thread per box
		active_thread_number = NUM_THREADS;
	else
		active_thread_number = number_of_boxes;	
	pthread_barrier_init(&barrier1, NULL, active_thread_number);
	pthread_barrier_init(&barrier2, NULL, active_thread_number);
	
        for(long tn=0;tn<active_thread_number-1;tn++) // create threads to process boxes
        {
		th_arg *thread_args;
		thread_args = new th_arg;
		pair<long,long> * arguments;
	    	arguments = new pair<long,long>;
	    	arguments->first = tn*factor;
	    	arguments->second = (tn+1)*factor-1;
 		thread_args->tid =tn;
		//cout <<"here"<<endl;
		thread_args->arguments =  arguments;

		// = new th_arg(arguments,tn);
	    
	    //cout<<tn<<" L: "<<arguments->first<<" H: "<<arguments->second<<endl;
	   
            rc = pthread_create(&threads[tn], NULL,calc_updates, (void *)thread_args);
            if (rc){
                        cout<<"ERROR; return code from pthread_create() is "<< rc<<endl;
                        exit(-1);
                }
        }
	pair<long,long> * arguments;
	arguments = new pair<long,long>;
	arguments->first = (active_thread_number-1)*factor;
	arguments->second = number_of_boxes-1;
	th_arg *thread_args = new th_arg;
	thread_args->arguments = arguments;
	thread_args->tid = active_thread_number-1;
	
	//cout<<(active_thread_number-1)<<" L: "<<arguments->first<<" H: "<<arguments->second<<endl;
        rc = pthread_create(&threads[(active_thread_number-1)], NULL,calc_updates, (void *)thread_args);
        if (rc){
                    cout<<"ERROR; return code from pthread_create() is "<< rc<<endl;
                    exit(-1);
               }
       for(long tn=0; tn<active_thread_number; tn++) // join threads 
       {
            rc = pthread_join(threads[tn], &th_status);
            if (rc) {
                cout<<"ERROR; return code from pthread_join() is"<<rc;
                exit(-1);
            }
            //cout<<"Completed join with thread"<<t<<" having a status of"<<(long)th_status<<endl;
       }
        
   /* do // this is the "repeat until converged" loop
    {
        //cout<<"=================start iteration:"<<iterations<<endl;
        long active_thread_number;
	if(number_of_boxes>=NUM_THREADS) // at maximum create only one thread per box
		active_thread_number = NUM_THREADS;
	else
		active_thread_number = number_of_boxes;
	//cout<<"active_thread_number: "<<active_thread_number<<endl;
	pthread_barrier_init(&barrier1, NULL, active_thread_number);
	pthread_barrier_init(&barrier2, NULL, active_thread_number);
        for(long tn=0;tn<active_thread_number-1;tn++) // create threads to process boxes
        {
	    pair<long,long> * arguments;
	    arguments = new pair<long,long>;
	    arguments->first = tn*factor;
	    arguments->second = (tn+1)*factor-1;
	    //cout<<tn<<" L: "<<arguments->first<<" H: "<<arguments->second<<endl;
	    
            rc = pthread_create(&threads[tn], NULL,calc_updates, (void *)arguments);
            if (rc){
                        cout<<"ERROR; return code from pthread_create() is "<< rc<<endl;
                        exit(-1);
                }
        }
	
	pair<long,long> * arguments;
	arguments = new pair<long,long>;
	arguments->first = (active_thread_number-1)*factor;
	arguments->second = number_of_boxes-1;
	//cout<<(active_thread_number-1)<<" L: "<<arguments->first<<" H: "<<arguments->second<<endl;
        rc = pthread_create(&threads[(active_thread_number-1)], NULL,calc_updates, (void *)arguments);
        if (rc){
                    cout<<"ERROR; return code from pthread_create() is "<< rc<<endl;
                    exit(-1);
               }
       for(long tn=0; tn<active_thread_number; tn++) // join threads 
       {
            rc = pthread_join(threads[tn], &th_status);
            if (rc) {
                cout<<"ERROR; return code from pthread_join() is"<<rc;
                exit(-1);
            }
            //cout<<"Completed join with thread"<<t<<" having a status of"<<(long)th_status<<endl;
       }
       //commite_updates();
       iterations++;
    } while (!converged(epsilon));// && iterations<5);*/

}
int main(int argc, char** argv) 
{
    cout<<"Usage: ./disposal affect_rate epsilon num_threads < input_test_file\n";
    cout<<"***************************************************************\n";
    AFFECT_RATE = atof(argv[1]);
    Epsilon=atof(argv[2]);
    NUM_THREADS =atoi(argv[3]);
    cout<<"start reading file...\n";
    read_grid_file();
    cout<<"Start!; it may take up to 4 minutes..."<<endl;
    auto t_chrono = chrono::system_clock::now();
    time_t timer1, timer2;
    time(&timer1);
    clock_t t;
    t = clock();
    dissipate();
    t = clock() - t;
    time(&timer2);
    double seconds = timer2-timer1;
    auto t_chronod = chrono::system_clock::now() - t_chrono;
    //cout<<"dissipation converged in "<< iterations<<" iterations,\n"; 
    cout<<"\t with max DSV = "<<max_dsv<<"\t and min DSV = "<<min_dsv<<endl;
    cout<<"\t affect rate = "<<AFFECT_RATE<<";\t\tepsilon = "<<Epsilon<<endl<<endl;
    cout<<"elapsed convergence loop time (clock):"<<t<<endl;
    cout<<"elapsed convergence loop time (time):"<<seconds<<endl;
    cout<<"elapsed convergence loop time (chrono):"<< chrono::duration<double,std::milli>(t_chronod).count()<<endl;
    cout<<"***********************************************************************"<<endl;
    pthread_exit(NULL);
    return 0;
}

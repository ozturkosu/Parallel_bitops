#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <papi.h>
#define N 4096
#define Niter 10
#define threshold 0.0000001

#define PAPI_ERROR_CHECK(X) \
        if((X)!=PAPI_OK) \
    {fprintf(stderr,"PAPI Error \n"); exit(-1);}

// To run with PAPI
// Use "gcc -O3 -DENABLE_PAPI -lpapi " to compile with GNU gcc
// Use "icc -O3 --DENABLE_PAPI -lpapi " to compile with Intel icc
//
// To get vectorization report use the following:
// gcc -O3 -ftree-vectorizer-verbose=2 filename.c >& filename.gccvecrpt
// icc -O3 -qopt-report=2 -qopt-report-phase=vec filename.c ; Report will be placed in filename.optrpt
//
// Using PAPI and vectorization getting report can of course be combined

double A[N][N], x[N],y[N],z[N],yy[N],zz[N];
int main(){
double rtclock();

void papi_print_helper(const char* msg, long long *values);

void pa1p1(int n, double m[][n], double x[n], double y[n], double z[n]);
void pa1p1opt(int n, double m[][n], double x[n], double y[n], double z[n]);
void compare(int n, double wref[n], double w[n]);

double clkbegin, clkend;
double t;
double rtclock();

int i,j,it;

  for(i=0;i<N;i++)
   { 
     x[i] = i; 
     y[i]= 0; z[i] = 1.0;
     yy[i]= 0; zz[i] = 1.0;
     for(j=0;j<N;j++) A[i][j] = (i+2.0*j)/(2.0*N);
   }

#ifdef ENABLE_PAPI
    int event_set = PAPI_NULL;
    PAPI_library_init(PAPI_VER_CURRENT);
    PAPI_ERROR_CHECK(PAPI_create_eventset(&event_set));
    PAPI_ERROR_CHECK(PAPI_add_event(event_set, PAPI_DP_OPS));
    PAPI_ERROR_CHECK(PAPI_add_event(event_set, PAPI_VEC_DP));
    PAPI_ERROR_CHECK(PAPI_add_event(event_set, PAPI_L3_TCM));
    PAPI_ERROR_CHECK(PAPI_add_event(event_set, PAPI_RES_STL));
    long_long papi_values[4];
    PAPI_ERROR_CHECK(PAPI_start(event_set));
#endif

  clkbegin = rtclock();
  for(it=0;it<Niter;it++) pa1p1(N,A,x,y,z);
  clkend = rtclock();
#ifdef ENABLE_PAPI
    PAPI_ERROR_CHECK(PAPI_stop(event_set, papi_values));
    papi_print_helper("Base Version",papi_values);
#endif
  t = clkend-clkbegin;
  if (y[N/2]*y[N/2] < -100.0) printf("%f\n",y[N/2]);
  printf("Problem 1 Reference Version: Matrix Size = %d; %.2f GFLOPS; Time = %.3f sec; \n",
          N,4.0*1e-9*N*N*Niter/t,t);

  for(i=0;i<N;i++)
   {
     yy[i]= 0; zz[i] = 1.0;
   }
#ifdef ENABLE_PAPI
    PAPI_ERROR_CHECK(PAPI_start(event_set));
#endif
  clkbegin = rtclock();
  for(it=0;it<Niter;it++) pa1p1opt(N,A,x,yy,zz);
  clkend = rtclock();
  t = clkend-clkbegin;
#ifdef ENABLE_PAPI
    PAPI_ERROR_CHECK(PAPI_stop(event_set, papi_values));
    papi_print_helper("Optimized Version",papi_values);
#endif
  if (yy[N/2]*yy[N/2] < -100.0) printf("%f\n",yy[N/2]);
  printf("Problem 1 Optimized Version: Matrix Size = %d; %.2f GFLOPS; Time = %.3f sec; \n",
          N,4.0*1e-9*N*N*Niter/t,t);
  compare(N,y,yy);

}

void pa1p1(int n, double m[][n], double x[n], double y[n], double z[n])
{ int i,j;
  for(i=0;i<n;i++)
    for(j=0;j<n;j++)
    {
      y[j] = y[j] + m[i][j]*x[i];
      z[j] = z[j] + m[j][i]*x[i];
    }
}

void pa1p1opt(int n, double m[][n], double x[n], double y[n], double z[n])
// Initially identical to reference; make your changes to optimize this code
{ int i,j;
  for(i=0;i<n;i++)
    for(j=0;j<n;j++)
    {
      y[j] = y[j] + m[i][j]*x[i];
      z[j] = z[j] + m[j][i]*x[i];
    }
}


double rtclock()
{
  struct timezone Tzp;
  struct timeval Tp;
  int stat;
  stat = gettimeofday (&Tp, &Tzp);
  if (stat != 0) printf("Error return from gettimeofday: %d",stat);
  return(Tp.tv_sec + Tp.tv_usec*1.0e-6);
}

void compare(int n, double wref[n], double w[n])
{
double maxdiff,this_diff;
int numdiffs;
int i;
  numdiffs = 0;
  maxdiff = 0;
  for (i=0;i<n;i++)
    {
     this_diff = wref[i]-w[i];
     if (this_diff < 0) this_diff = -1.0*this_diff;
     if (this_diff>threshold)
      { numdiffs++;
        if (this_diff > maxdiff) maxdiff=this_diff;
      }
    }
   if (numdiffs > 0)
      printf("%d Diffs found over threshold %f; Max Diff = %f\n",
               numdiffs,threshold,maxdiff);
   else
      printf("No differences found between base and test versions\n");
}

void papi_print_helper(const char* msg, long long *values)
{
    printf("\n=====================PAPI COUNTERS==========================\n\n");
    printf("(%s): DP operations : %.2f G\n",          msg, values[0]*1e-9);
    printf("(%s): DP vector instructions : %.2f M\n", msg, values[1]*1e-6);
    printf("(%s): L3 cache misses : %.2f M\n",              msg, values[2]*1e-6);
    printf("(%s): Resource Stall Cycles: %.2f M\n",         msg, values[3]*1e-6);
    printf("=================PAPI COUNTERS END==========================\n\n");
}


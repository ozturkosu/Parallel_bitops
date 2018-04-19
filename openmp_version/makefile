all: main
.PHONY : clean 
main: main.cc
	g++ *.cc -std=gnu++0x -o proj -fopenmp -lrt -O3 

clean:
	rm -rf *.o *.out proj

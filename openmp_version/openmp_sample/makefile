all: disposable persistent
.PHONY : clean 
disposable: 
	g++ Asudeh_Omid_disposable.cc -std=gnu++0x  -O3 -lrt  -fopenmp -o disposable  
persistent: 
	g++ Asudeh_Omid_persistent.cc -std=gnu++0x -o persistent -O3 -lrt  -lpthread -fopenmp 
clean:
	rm -rf *.o *.out disposable persistent

all: main
.PHONY : clean 
main: main.cu
	nvcc -O -o proj main.cu 
clean:
	rm -rf *.o *.out 

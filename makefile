all: main
.PHONY : clean 
main: main.cu
	nvcc -O -o proj  *.cu 
clean:
	rm -rf *.o *.out 

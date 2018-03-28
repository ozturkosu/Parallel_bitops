all: lab_4p1 lab_4p2
#all: lab4p1_cuda
.PHONY : clean 
lab_4p1: 
	nvcc -O -o lab4p1 Asudeh_omid_lab4p1.cu     
lab_4p2: 
	nvcc -O -o lab4p2 Asudeh_omid_lab4p2.cu  bmp_reader.o
clean:
	rm -rf *.o *.out

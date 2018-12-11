#
# Makefile
#


CUCSRSRC=main.cu

CPPSRC=mybitops.cc

EXECCSR=main

OBJSCSR=$(CUCSRSRC:.cu=.o)
OBJSCSR+=$(CPPSRC:.cc=.o)


NVCCFLAGS=--gpu-architecture=compute_35 --gpu-code=sm_35 -O3

CC=nvcc
HCC=gcc

all: $(EXECCSR)

$(EXECCSR): $(OBJSCSR)
	$(CC) $(NVCCFLAGS) $^ -o $@


%.o : %.cu
	$(CC)  $(NVCCFLAGS) -c $< -o $@

%.o : %.cpp
	$(HCC) -O3 -c $< -o $@

clean:
	rm -f $(EXECCSR) $(OBJSCSR)


# vim:ft=make
#

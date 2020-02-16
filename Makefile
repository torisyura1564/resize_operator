CC = g++
CFLAG = -O4  -funroll-loops -g -Wall

SMCOBJ = resize_operator.o
SMC = MKSMC

LDFLAGS =`pkg-config opencv --libs`

all : $(SMC)

$(SMC) : $(SMCOBJ)
	$(CC) $(CFLAG) $(LDFLAGS) -std=c++11 -o $(SMC) $(SMCOBJ) -lm

.cpp.o :
	$(CC) $(CFLAG) -std=c++11 -c $<

resize_operator.o : resize_operator.cpp new_smc.h

clean :
	rm -f $(SMC) $(SMCOBJ) core.* *~
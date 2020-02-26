CC = g++
CFLAG = -O4  -funroll-loops -g -Wall

SMCOBJ = Multi-Operator.o
SMC = MKSMC
SCLOBJ = scaling.o
SCL = MKSCL

LDFLAGS =`pkg-config opencv --libs`

all : $(SMC) $(SCL)

$(SMC) : $(SMCOBJ)
	$(CC) $(CFLAG) $(LDFLAGS) -std=c++11 -o $(SMC) $(SMCOBJ) -lm

$(SCL) : $(SCLOBJ)
	$(CC) $(CFLAG) $(LDFLAGS) -std=c++11 -o $(SCL) $(SCLOBJ) -lm

.cpp.o :
	$(CC) $(CFLAG) -std=c++11 -c $<


Multi-Operator.o : Multi-Operator.cpp new_smc.h
scaling.o : scaling.cpp

clean :
	rm -f $(SMC) $(SMCOBJ) $(SCL) $(SCLOBJ) core.* *~
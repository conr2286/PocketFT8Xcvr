#Makefile for test5351

CXX=g++
CXXFLAGS=-O -std=c++17 -Wall -g

OBJS=test5351.o Pi2CBlock.o SI5351a.o Pi2C.o
HDRS=SI5351a.h Pi2C.h Pi2CBlock.h

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

all:	test5351
 
test5351: $(OBJS) $(HDRS) Makefile
	$(CXX) $(CXXFLAGS) $(OBJS) -o test5351

clean:
	rm -f $(OBJS) test5351
	

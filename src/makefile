
CXX=g++
CFLAGS=-Wall -g
LIB=-lncurses

pic_programmer: main.o rs232.o hex.o
	$(CXX) -o $@ main.o rs232.o hex.o conio.h $(LIB)
	
main.o: main.cc
	$(CXX) $(CFLAGS) -c -o $@ main.cc

hex.o: hex.cc hex.h
	$(CXX) $(CFLAGS) -c -o $@ hex.cc
	
rs232.o: rs232.cc rs232.h
	$(CXX) $(CFLAGS) -c -o $@ rs232.cc



clean:
	rm -f *.o
	
all:
	make

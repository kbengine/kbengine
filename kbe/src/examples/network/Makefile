objects = main.o

network : $(objects)
	g++ -o a $(objects)
main.o : main.cpp main.hpp
	g++ -c -O3 -Wall -W -m32 main.cpp 
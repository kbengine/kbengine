objects = a.o

a : $(objects)
	g++ -o a $(objects)
a.o : a.cpp a.hpp
	g++ -c -O3 -Wall -W -m32 a.cpp 
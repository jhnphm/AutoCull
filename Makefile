autocull: autocull.cpp
	g++ -Wall -O0 -std=c++11 -ggdb3 autocull.cpp `pkg-config --libs exiv2` -ljpeg -o autocull

clean:
	rm autocull

all: autocull

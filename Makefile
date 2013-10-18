


autocull: autocull.cpp
	g++ -Wall -O2 -std=c++11 autocull.cpp `pkg-config --libs exiv2` -ljpeg -o autocull

autocull_dbg: autocull.cpp
	g++ -pg -Wall -O0 -std=c++11 -ggdb3 autocull.cpp `pkg-config --libs exiv2` -ljpeg -o autocull_dbg

autocull_prof: autocull.cpp
	g++ -pg -Wall -O2 -std=c++11 -ggdb3 autocull.cpp `pkg-config --libs exiv2` -ljpeg -o autocull_prof

clean:
	rm autocull autocull_dbg autocull_prof

all: autocull

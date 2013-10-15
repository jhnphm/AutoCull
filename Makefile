autocull: autocull.cpp
	g++ -ggdb3 autocull.cpp `pkg-config --libs exiv2` -o autocull

clean:
	rm autocull

all: autocull

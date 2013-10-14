autocull: autocull.c
	gcc -ggdb3 -std=c99 autocull.c -lraw -lgomp -lexif -o autocull

clean:
	rm autocull

all: autocull

CFLAGS := -std=c++11 -Wall

solve: maze
	./maze maze.txt

all: maze

clean:
	rm -f maze

maze: main.cc
	$(CXX) -o $@ $(CFLAGS) $<

.PHONY: all clean solve

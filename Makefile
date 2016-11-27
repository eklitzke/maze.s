CFLAGS := -std=c++11 -Wall

maze: main.cc
	$(CXX) -o $@ $(CFLAGS) $<

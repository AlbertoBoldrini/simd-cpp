

all: example.cpp ../simd.hpp
	g++ --std=c++14 -Ofast -march=native -g example.cpp -o example
	g++ --std=c++14 -Ofast -march=native -g -S example.cpp -o example.S
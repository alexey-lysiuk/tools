all : unpack

unpack : unpack.cpp
	g++ -o $@ $< -g -lz -lbz2

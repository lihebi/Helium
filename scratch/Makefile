a.out: stack-vs-heap.cc
	g++ -g --std=c++11 $<
valgrind-parser: valgrind.cc
	g++ -g --std=c++11 $< -lpugi -o valgrind-parser



z3-test: z3-test.cc
	g++ -lz3 z3-test.cc

.PHONY: all test clean

.PHONY: all test compile_test
all: plugin.so

INCLUDE=-I/home/hebi/codesurfer-2.2p0/csurf/include
CFLAGS=--std=c++11

%.so: %.c
	gcc -shared $< -o $@ -fPIC -O

# Not able to make the C++ version work.
# Use the C version instead
%.so: %.cc
	g++ $(CFLAGS) $(INCLUDE) -shared $< -o $@ -fPIC -O
test: all
	csurf -nogui -l `pwd`/plugin `pwd`/test/myproj

clean:
	rm -rf *.so

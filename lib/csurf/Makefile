.PHONY: all test clean

.PHONY: all test compile_test
all: plugin.so

%.so: %.c
	gcc -shared $< -o $@ -fPIC -O

test: all
	csurf -nogui -l `pwd`/plugin `pwd`/test/myproj

clean:
	rm -rf *.so

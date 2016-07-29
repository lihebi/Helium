SRC=pugixml.cpp
OBJ=pugixml.o
HEADERS=pugixml.hpp pugiconfig.hpp
REAL_NAME=libpugixml.so.1
SO_NAME=libpugixml.so
CC=g++
all: $(REAL_NAME)

$(REAL_NAME):
	$(CC) -Wall -fPIC -c $(SRC)
	$(CC) -shared -Wl,-soname,$(SO_NAME) -o $(REAL_NAME) $(OBJ)

install: $(REAL_NAME)
	cp $(REAL_NAME) /usr/local/lib
	cp $(HEADERS) /usr/local/include

clean:
	rm -rf *.o
	rm -rf *.so
	rm -rf $(REAL_NAME)

.PHONY: all clean install

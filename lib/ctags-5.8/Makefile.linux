SRC=readtags.c
OBJ=readtags.o
HEADERS=readtags.h
REAL_NAME=libctags.so.1
SO_NAME=libctags.so
CC=gcc
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

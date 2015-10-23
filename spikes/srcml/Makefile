SRC=$(wildcard *.cpp)
OBJ=$(SRC:.cpp=.xml)

all: $(OBJ)

%.xml: %.cpp
	src2srcml --position $< > $@

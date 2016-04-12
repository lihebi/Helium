CC := g++
SRCDIR := .
BUILDDIR := build
SRCEXT := cc
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
HEADERS := $(shell find $(SRCDIR) -type f -name "*.h")
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
DEP = $(OBJECTS:%.o=%.d)
HELIUM_ROOT := ../..
LIB_HELIUM := $(HELIUM_ROOT)/bin/libhelium.a
CFLAGS := -g -Wall --std=c++11 -I$(HELIUM_ROOT)/src 
C_LIB := -lboost_program_options -lboost_system -lboost_filesystem -lboost_regex
C_LIB += -lpugi -lctags # 3rd party library, shipped with source code
C_LIB += -lgtest -lsqlite3
TARGET := mlslice

all: $(TARGET)

copy: $(TARGET)
	cp $(TARGET) $(HELIUM_ROOT)/bin

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $(OBJECTS) $(LIB_HELIUM)  $(C_LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -c -o $@ $<

clean:
	rm -rf build
	rm -rf mlslice

# FIXME the --benchmark must not contain ~/xxx
test:
	./mlslice --slice=data/32.txt --benchmark=/Users/hebi/benchmark/gzip-1.7

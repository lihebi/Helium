#
# TODO: Move `libmongoclient.a` to /usr/local/lib so this can work on production servers
#

CC := g++
# CC := clang --analyze # and comment out the linker last line for sanity
SRCDIR := src
SPIKE_DIR := spikes
BUILDDIR := build
TARGET := bin/helium
BIN_DIR := bin

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
SPIKE_SRCS := $(shell find $(SPIKE_DIR) -type f -name *.$(SRCEXT))
SPIKE_OBJECTS := $(patsubst $(SPIKE_DIR)/%,$(BIN_DIR)/%,$(SPIKE_SRCS:.$(SRCEXT)=))
CFLAGS := -g -Wall --std=c++11
# -pthread
LIB := -lboost_program_options -lboost_system -lboost_filesystem -lpugi -lctags
INC := -I include

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

install: $(TARGET)
	cp $(TARGET) /usr/local/bin

clean:
	$(RM) -r $(BUILDDIR) $(TARGET)
	$(RM) -rf $(BIN_DIR)/*

# Tests
tester:
	$(CC) $(CFLAGS) test/tester.cpp $(INC) $(LIB) -o bin/tester

lib:
	g++ -dynamiclib -undefined suppress -flat_namespace *.o -o libpugi.dylib
	g++ -dynamiclib -o libpugi.dylib pugixml.dylib
	# then move it to /usr/local/lib, or cannot build

# Spikes
# ticket:
# 	$(CC) $(CFLAGS) spikes/ticket.cpp $(INC) $(LIB) -o bin/ticket
# path:
# 	$(CC) $(CFLAGS) spikes/testpath.cpp $(INC) $(LIB) -o bin/path
spike: $(SPIKE_OBJECTS)

$(BIN_DIR)/%: $(SPIKE_DIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< $(INC) $(LIB) -o $@

BOOST_INCLUDE := filesystem system program_options
BOOST_INCLUDE_PATH := $(addprefix /usr/local/include/boost/, $(BOOST_INCLUDE))

.PHONY: clean

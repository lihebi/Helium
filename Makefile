#
# TODO: Move `libmongoclient.a` to /usr/local/lib so this can work on production servers
#

CC := g++
# CC := clang --analyze # and comment out the linker last line for sanity

##############################
## Dir settings
##############################
SRCDIR := src
SPIKE_DIR := spikes
BUILDDIR := build
TARGET := bin/helium
BIN_DIR := bin

##############################
## Source and Dist
##############################
SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
# test source
# the file xx/xx/test_xxx.cpp is recognized as test file.
TEST_SOURCES := $(shell find $(SRCDIR) -type f -name *test_*.$(SRCEXT))
# test files are filtered out from SOURCES
SOURCES := $(filter-out $(TEST_SOURCES), $(SOURCES))

# objects
# pattern substring replacement: $(patsubst pattern,replacement,text)
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
TEST_OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(TEST_SOURCES:.$(SRCEXT)=.o))

TEST_MAIN := test/test_main.cpp
TEST_TARGET := bin/test

# get lib name for Mac OS and Linux
SONAME :=
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Darwin)
	SONAME := libhelium.dylib
endif
ifeq ($(UNAME_S), Linux)
	SONAME := libhelium.so.1
endif

TARGET_LIB := $(BUILDDIR)/$(SONAME)

CLIENT_SRC := client/main.cpp # main.cpp is separate from other source files because I need to have a dynamic lib for test to link

##############################
## Compile flags
##############################

CFLAGS := -g -Wall --std=c++11

DYLIB_FLAG :=
ifeq ($(UNAME_S), Darwin)
	DYLIB_FLAG := -dynamiclib
endif
ifeq ($(UNAME_S), Linux)
	DYLIB_FLAG := -shared -Wl,-soname,$(SONAME)
endif

# -pthread
C_TEST_LIB := -L$(BUILDDIR) -lhelium # the target helium library
C_TEST_LIB += -lgtest # google test framework

C_TEST_LIB += -lboost_unit_test_framework # boost test framework
C_LIB := -lboost_program_options -lboost_system -lboost_filesystem -lboost_regex # other boost libraries used in Helium
C_LIB += -lpugi -lctags # 3rd party library, shipped with source code
C_INC := -I include # local include folder

##############################
## Targets
##############################
.PHONY: all clean doc libhelium test install

all: client

# libhelium.so is only used for test
libhelium: $(TARGET_LIB)

$(TARGET_LIB): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(DYLIB_FLAG) $(C_LIB) $(C_INC) -o $(TARGET_LIB) $(OBJECTS)


client: $(TARGET)
# Compile client based on the object files, instead of the dynamic lib
$(TARGET): $(CLIENT_SRC) $(OBJECTS)
	$(CC) $(C_LIB) $(C_INC) -o $@ $(CLIENT_SRC) $(OBJECTS)

test: libhelium $(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJECTS) $(TEST_MAIN)
	$(CC) $(CFLAGS) $(C_INC) $(C_LIB) $(C_TEST_LIB) -o $@ $^

# For all target build/*.o, find the counter part in src/*.cpp
# compile it into the target
$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(C_INC) -c -o $@ $<



doc:
	doxygen

install: $(TARGET)
	cp $(TARGET) /usr/local/bin

clean:
	$(RM) -r $(BUILDDIR) $(TARGET)
	$(RM) -rf $(BIN_DIR)/*

systype.tags:
	ctags -f systype.tags --exclude=boost\
		--exclude=llvm\
		--exclude=c++\
		--exclude=linux\
		--exclude=xcb\
		--exclude=X11\
		--exclude=openssl\
		--exclude=xorg\
		-R /usr/include/ /usr/local/include

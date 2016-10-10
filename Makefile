#
# TODO: Move `libmongoclient.a` to /usr/local/lib so this can work on production servers
#

CC := g++
# Clang is not compitible with g++ ABI, which causes the boost::program_option::args not defined, on Debian stretch
# CC := clang++-3.8
# CC := clang --analyze # and comment out the linker last line for sanity

##############################
## Dir settings
##############################
SRCDIR := src
BUILDDIR := build
BIN_DIR := bin

TARGET := bin/helium
TEST_TARGET := bin/test

SRCEXT := cc
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
HEADERS := $(shell find $(SRCDIR) -type f -name "*.h")

MAIN := $(SRCDIR)/main.cc
TEST_MAIN := $(SRCDIR)/test_main.cc

SOURCES := $(filter-out $(MAIN) $(TEST_MAIN), $(SOURCES))
# pattern substring replacement: $(patsubst pattern,replacement,text)
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

# get lib name for Mac OS and Linux
# SONAME :=
# UNAME_S := $(shell uname -s)
# ifeq ($(UNAME_S), Darwin)
# 	SONAME := libhelium.dylib
# endif
# ifeq ($(UNAME_S), Linux)
# 	SONAME := libhelium.so.1
# endif

# TARGET_LIB := $(BUILDDIR)/$(SONAME) # This is the libhelium filename

DEP = $(OBJECTS:%.o=%.d)





##############################
## Compile flags
##############################

CFLAGS := -g -Wall --std=c++11 # -fsanitize=address

# DYLIB_FLAG :=
# ifeq ($(UNAME_S), Darwin)
# 	DYLIB_FLAG := -dynamiclib
# endif
# ifeq ($(UNAME_S), Linux)
# 	DYLIB_FLAG := -shared -Wl,-soname,$(SONAME)
# endif

# -pthread
# C_TEST_LIB := -L$(BUILDDIR) -lhelium # the target helium library
# C_TEST_LIB += -lgtest # google test framework
# C_TEST_LIB += -lboost_unit_test_framework # boost test framework

C_INCLUDE := -Isrc
C_LIB := -pthread
# C_LIB += -L/usr/lib -L/usr/local/lib
C_LIB += -lboost_program_options -lboost_system -lboost_filesystem -lboost_regex -lboost_timer # other boost libraries used in Helium
C_LIB += -lpugixml
C_LIB += -lctags # 3rd party library, shipped with source code
C_LIB += -lgtest -lgtest_main
C_LIB += -lsqlite3
# C_LIB += -pg # gprof profile. Will make the program run slower

##############################
## Targets
##############################
.PHONY: all clean doc libhelium test install tmp depend utils

all: client

# Include all .d files
-include $(DEP)


##############################
## Helium executable
##############################

# the actual execuable of Helium
client: $(TARGET) # $(TEST_TARGET)
# Compile client based on the object files, instead of the dynamic lib
# Always put the C_LIB at the end, AFTER the object that uses it.
# Otherwise the compiler gives you undefined reference when linking
# This is a common trick on linux, and mac can do with both
$(TARGET): $(MAIN) $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(C_INCLUDE) -o $@ $(MAIN) $(OBJECTS)  $(C_LIB)

LIB_HELIUM := bin/libhelium.a
lib: $(LIB_HELIUM)

# archive object files into libhelium.a
$(LIB_HELIUM): $(OBJECTS)
	@mkdir -p $(dir $@)
# use -cvq is much faster, but the object code is appended, not replaced
# thus removal of the lib file is required. Don't want to do that.
	ar -cvr $@ $(OBJECTS)




##############################
## Helium Utilities
##############################

utils:
	cd utils/mlslice; make

##############################
## tests
##############################
# libhelium.so is only used for test
# libhelium: $(TARGET_LIB)
# $(TARGET_LIB): $(OBJECTS)
# 	@mkdir -p $(dir $@)
# 	$(CC) $(DYLIB_FLAG) $(C_LIB) -o $(TARGET_LIB) $(OBJECTS)

# test: libhelium $(TEST_TARGET)

# $(TEST_TARGET): $(TEST_OBJECTS) $(TEST_MAIN)
# 	$(CC) $(CFLAGS) $(C_LIB) $(C_TEST_LIB) -o $@ $^

test: $(TEST_TARGET)

dotest: test
	$(TEST_TARGET)

#	$(CC) -o $@ $(TEST_MAIN) $(OBJECTS)  $(C_LIB)
#	$(CC) -o $@ $^  $(C_LIB)
$(TEST_TARGET): $(TEST_MAIN) $(OBJECTS)
	$(CC) $(C_INCLUDE) -o $@ $^ $(CFLAGS) $(C_LIB)

##############################
## General compile rule
##############################

# For all target build/*.o, find the counter part in src/*.cpp
# compile it into the target
$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(C_INCLUDE) $(CFLAGS) $(C_LIB) -MMD -c -o $@ $<


# %.d: %.cc ${SOURCES}
# 	$(CC) -MM $< -o $@

# -include ${SOURCES:.cc=.d}

##############################
## other trival staff
##############################

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

bootstrap: systype.tags
	cp helium.conf.sample helium.conf

# this is what every that is handy
tmp: $(TARGET)
	helium test/benchmark

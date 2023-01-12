
# Compilers etc.

CXX=g++
AR=ar
SH=g++
LD=g++

# Sources
SRC_DIR := src
TEST_DIR := test
EXAMPLE_DIR := example
BUILD_DIR := build

OBJ_DIR := $(BUILD_DIR)/obj
A_OBJ_DIR := $(OBJ_DIR)/a
SO_OBJ_DIR := $(OBJ_DIR)/so
TEST_OBJ_DIR := $(OBJ_DIR)/test
EXAMPLE_OBJ_DIR := $(OBJ_DIR)/example

LIB_DIR := $(BUILD_DIR)/lib
EXAMPLE_OUT_DIR := $(BUILD_DIR)/example
TEST_OUT_DIR := $(BUILD_DIR)/test
A_LIB := $(LIB_DIR)/libsimple_http.a
SO_LIB := $(LIB_DIR)/libsimple_http.so

SRC_FILES := $(wildcard $(SRC_DIR)/*/*.cpp)
SRC_FILES += $(wildcard $(SRC_DIR)/*/*/*.cpp)
A_OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(A_OBJ_DIR)/%.o,$(SRC_FILES))
SO_OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(SO_OBJ_DIR)/%.o,$(SRC_FILES))


# Flags

example_CXXFLAGS=-m64 -fvisibility=hidden -fvisibility-inlines-hidden -O3 -std=c++20 -Iinclude -Isrc -DNDEBUG
example_LDFLAGS=-m64 -L$(LIB_DIR) -s -lsimple_http -lpthread

shared_lib_CXXFLAGS=-m64 -fPIC -O3 -std=c++20 -Iinclude -Isrc -DNDEBUG
shared_lib_SHFLAGS=-shared -fPIC -m64 -s

static_lib_CXXFLAGS=-m64 -fvisibility=hidden -fvisibility-inlines-hidden -O3 -std=c++20 -Iinclude -Isrc -DNDEBUG
static_lib_ARFLAGS=-cr

test_CXXFLAGS=-m64 -fvisibility=hidden -fvisibility-inlines-hidden -O3 -std=c++20 -Iinclude -Isrc -DNDEBUG
test_LDFLAGS=-m64 -L$(LIB_DIR) -s -lsimple_http -lpthread

# Rules

.PHONY: all

all: lib tests examples

## Exmaples

examples: example_http_server example_tcp_server

example_http_server: $(EXAMPLE_OBJ_DIR)/http_server.o $(A_LIB)
	@mkdir -p $(EXAMPLE_OUT_DIR)
	$(LD) -o $(EXAMPLE_OUT_DIR)/$@ $< $(example_LDFLAGS)

example_tcp_server: $(EXAMPLE_OBJ_DIR)/tcp_server.o $(A_LIB)
	@mkdir -p $(EXAMPLE_OUT_DIR)
	$(LD) -o $(EXAMPLE_OUT_DIR)/$@ $< $(example_LDFLAGS) 

$(EXAMPLE_OBJ_DIR)/%.o: $(EXAMPLE_DIR)/%.cpp
	@echo "Compiling $<" ...
	@mkdir -p $(@D)
	$(CXX) $(example_CXXFLAGS) -c -o $@ $<

## Libraries

lib: shared_lib static_lib

static_lib: $(A_LIB)

$(A_LIB): $(A_OBJ_FILES)
	@echo Linking simple_http.a
	@mkdir -p $(@D)
	$(AR) $(static_lib_ARFLAGS) $(A_LIB) $^

$(A_OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Compiling $<" ...
	@mkdir -p $(@D)
	$(CXX) $(static_lib_CXXFLAGS) -c -o $@ $<

shared_lib: $(SO_LIB)

$(SO_LIB): $(SO_OBJ_FILES)
	@echo Linking simple_http.so
	@mkdir -p $(@D)
	$(SH) -o $(SO_LIB) $^ $(shared_lib_SHFLAGS)

$(SO_OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Compiling $<" ...
	@mkdir -p $(@D)
	$(CXX) $(shared_lib_CXXFLAGS) -c -o $@ $<

## Tests

tests: msg_buffer_test

msg_buffer_test: $(TEST_OBJ_DIR)/msg_buffer_test.o $(A_LIB)
	@mkdir -p $(TEST_OUT_DIR)
	$(LD) $(test_LDFLAGS) -o $(TEST_OUT_DIR)/$@ $^

$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp
	@echo "Compiling $<" ...
	@mkdir -p $(@D)
	$(CXX) $(test_CXXFLAGS) -c -o $@ $<

clean:
	rm -rf build/obj
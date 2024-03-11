CC := g++
CFLAGS := -std=c++17 -Wall -O2 $(shell pkg-config --cflags sndfile)
LDFLAGS := $(shell pkg-config --libs sndfile)

TARGET := builds/SPConverter
SRC_DIR := src
SOURCE := $(wildcard $(SRC_DIR)/*.cpp)
LIB_DIR := src/includes/r8brain
LLIB_SRC := $(wildcard $(LIB_DIR)/*.cpp)
LIB_OBJ := $(patsubst $(LIB_DIR)/%.cpp, ../build/%.o, $(LIB_SRC))

all: $(TARGET)

$(TARGET): $(SOURCE) $(LLIB_SRC) $(LIB_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

../build/%.o: $(LIB_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf ../build $(TARGET)

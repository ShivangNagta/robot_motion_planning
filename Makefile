INC_DIR := include
SRC_DIR := src
OBJ_DIR := obj
LIBS_DIR := libs

CXX := g++
CXXFLAGS := -std=c++17 -g

# Only for UNIX currently
C_EXTERNAL_INCLUDE := $(shell pkg-config --cflags sdl2_ttf)
C_EXTERNAL_LIBS := $(shell pkg-config --libs sdl2_ttf)

CPPFLAGS := -I$(INC_DIR) $(C_EXTERNAL_INCLUDE)
# include LDLIBS for sdl2, i.e -lSDL2
LDFLAGS := -L$(LIBS_DIR) $(C_EXTERNAL_LIBS)

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
DEPS := $(wildcard $(INC_DIR)/*.hpp)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(SRC_DIR)/$(OBJ_DIR)/%.o)
TARGET := main

$(SRC_DIR)/$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm -f main $(OBJS)

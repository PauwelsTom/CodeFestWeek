# Compiler and linker
CXX = g++
LD = g++

# Directories
SRC_DIR = src
INCLUDE_DIR = include
INCLUDE_LIB_DIR = include_lib
LIB_DIR = lib

# Library
LIB_NAME = freenect_sync
LIB_VERSION = 0.7.5
LIB_FILE = $(LIB_DIR)/lib$(LIB_NAME).so.$(LIB_VERSION)

# Flags
CXXFLAGS = -I$(INCLUDE_DIR) 
CXXFLAGS += -I$(INCLUDE_LIB_DIR) 
CXXFLAGS += -I/usr/include/libusb-1.0 -Wall -Wextra -std=c++11
CXXFLAGS += -I/usr/include/opencv4


LDFLAGS = -L$(LIB_DIR) -lfreenect_sync
# LDFLAGS += -L$(LIB_DIR) -lfakenect
LDFLAGS += -L$(LIB_DIR) -lfreenect
LDFLAGS += -L/usr/lib -lusb-1.0
LDFLAGS += -lGL -lGLU -lglut

# LDFLAGS += -Lusr/local/lib -lopencv_core

LDFLAGS += `pkg-config --libs opencv4`

# Source files and object files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = main_program

# Rules
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(LD) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

.PHONY: all clean

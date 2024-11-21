# Compiler
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude -IC:/tools/vcpkg/vcpkg/installed/x64-windows/include

# Directories
SRC_DIR = src
CORE_DIR = Core
GEOM_DIR = Geometry
LIGHT_DIR = Lighting
MAT_DIR = Materials
UTIL_DIR = Utilities
IO_DIR = IO

# Source files and object files
SRCS = $(wildcard $(SRC_DIR)/*.cpp $(CORE_DIR)/*.cpp $(GEOM_DIR)/*.cpp \
               $(LIGHT_DIR)/*.cpp $(MAT_DIR)/*.cpp $(UTIL_DIR)/*.cpp $(IO_DIR)/*.cpp)
OBJS = $(SRCS:.cpp=.o)

# Executable name
TARGET = raytracer

# Build rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up object files and the executable
clean:
	-del /Q /F src\main.o Core\*.o Geometry\*.o Lighting\*.o Materials\*.o Utilities\*.o raytracer.exe 2>NUL

.PHONY: all clean
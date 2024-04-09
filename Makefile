# Define compiler
CXX = g++

# Get compiler flags for Python
PY_CFLAGS = $(shell python3-config --cflags)

# Get linker flags for Python
PY_LDFLAGS = $(shell python3-config --ldflags)

# Combine Python flags with your flags
CXXFLAGS = -Wall -std=c++11 $(PY_CFLAGS)
LFLAGS = $(PY_LDFLAGS) -lpigpio -lrt -pthread

# Define any directories containing header files other than /usr/include
INCLUDES =

# Define any libraries to link into executable
LIBS = -lpigpio -lrt -pthread

# Define the C++ source files
SRCS = robot_core.cpp camera_module.cpp motor_control.cpp

# Define the C++ object files 
OBJS = $(SRCS:.cpp=.o)

# Define the executable file 
MAIN = runapp

.PHONY: clean

all: $(MAIN)
	@echo Simple compiler named runapp has been compiled

$(MAIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

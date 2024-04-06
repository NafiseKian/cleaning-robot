# Define compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -std=c++11

# Define any directories containing header files other than /usr/include
INCLUDES = 

# Define library paths in addition to /usr/lib
LFLAGS = 

# Define any libraries to link into executable
LIBS = 

# Define the C++ source files
SRCS = robot_core.cpp camera_module.cpp

# Define the C++ object files 
OBJS = $(SRCS:.cpp=.o)

# Define the executable file 
MAIN = myApplication

.PHONY: clean

all: $(MAIN)
	@echo  Simple compiler named myApplication has been compiled

$(MAIN): $(OBJS) 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

# Define compiler
CXX = g++

# Combine Python flags with your flags
CXXFLAGS = -Wall -std=c++11 $(PY_CFLAGS)
LFLAGS = $(PY_LDFLAGS) -lpigpio -lgpiod -lrt -pthread

# Define any directories containing header files other than /usr/include
INCLUDES = -I/usr/include/eigen3

# Define any libraries to link into executable
LIBS = -lpigpio -lgpiod -lrt -pthread

# Define the C++ source files
SRCS = robot_core.cpp camera_module.cpp drive_module.cpp gps_module.cpp wifi_module.cpp ultra_sensor.h


# Define the C++ object files 
OBJS = $(SRCS:.cpp=.o)

# Define the executable file 
MAIN = runapp

.PHONY: all clean

all: $(MAIN)
	@echo Simple compiler named runapp has been compiled

$(MAIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

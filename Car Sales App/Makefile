# Makefile for Pqueue.cpp, Pqueue.h, and mytest.cpp

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -Wall -Wextra

# Source files
SRCS = dealer.cpp mytest.cpp

# Header files
HEADERS = dealer.h

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable name
EXEC = mytest

# Target: all (default target)
all: $(EXEC)

# Target: mytest (executable)
$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(EXEC)

# Target: %.o (object files)
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Target: clean (remove object files and executable)
clean:
	rm -f $(OBJS) $(EXEC)

# Target: rebuild (clean and build)
rebuild: clean all

# Target: run (run the executable)
run: $(EXEC)
	./$(EXEC)

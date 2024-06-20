# Compiler variables
CXX=g++
CXXFLAGS=-std=c++11 -Wall -g3 -c

# Object files
OBJS = main.o vaddr_tracereader.o Level.o PageTable.o log_helpers.o Bitstring.o

# Program name
PROGRAM = pagingwithpr

# Rules format:
# target : dependency1 dependency2 ... dependencyN
#     Command to make target, uses default rules if not specified

# First target is the one executed if you just type make
# make target specifies a specific target
# $^ is an example of a special variable.  It substitutes all dependencies

$(PROGRAM) : $(OBJS)
	$(CXX) -o $(PROGRAM) $^

main.o : main.cpp vaddr_tracereader.h Level.h PageTable.h Bitstring.h
	$(CXX) $(CXXFLAGS) main.cpp

vaddr_tracereader.o : vaddr_tracereader.cpp vaddr_tracereader.h
	$(CXX) $(CXXFLAGS) vaddr_tracereader.cpp

Level.o : Level.cpp Level.h
	$(CXX) $(CXXFLAGS) Level.cpp

PageTable.o : PageTable.cpp PageTable.h Level.h Bitstring.h
	$(CXX) $(CXXFLAGS) PageTable.cpp

log_helpers.o : log_helpers.cpp
	$(CXX) $(CXXFLAGS) log_helpers.cpp

Bitstring.o : Bitstring.cpp Bitstring.h
	$(CXX) $(CXXFLAGS) Bitstring.cpp

clean :
	rm -f *.o $(PROGRAM)

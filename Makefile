TARGET = convertDat2Root

SRCS = $(TARGET).cpp
OBJS = $(TARGET).o d2r.o Entry.o

ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS = $(shell root-config --libs)
ROOTGLIBS = $(shell root-config --glibs)

CXXFLAGS = $(ROOTCFLAGS) -Wall -Wextra -fPIC 
CXXLIBS = $(ROOTLIBS) -lSpectrum
CC = g++

$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) $(CXXLIBS) $(OBJS) -o $@

.cpp.o:
	$(CC) $(CXXFLAGS) -c $<

clean:
	rm -rf $(TARGET)
	rm -rf $(OBJS)

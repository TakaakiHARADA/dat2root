TARGET = dat2root

SRCS = $(TARGET).cpp
OBJS = $(TARGET).o

ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS = $(shell root-config --libs)
ROOTGLIBS = $(shell root-config --glibs)

CXXFLAGS = $(ROOTCFLAGS) -Wall -fPIC
CXXLIBS = $(ROOTLIBS) -lSpectrum
CXX = g++

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(CXXLIBS) $(OBJS) -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -rf $(TARGET)
	rm -rf $(OBJS)

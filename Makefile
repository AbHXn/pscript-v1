CXX = g++
CXXFLAGS = -O3 -std=c++17

SRC = $(shell find . -name "*.cpp")

OUT = pscript

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)

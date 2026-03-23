CXX = g++
CXXFLAGS = -std=c++20  -Iinclude -Wall

SRC = src/main.cpp \
      src/core/Commit.cpp \
			src/core/Branch.cpp \
			src/core/Repository.cpp \
      src/fileUtils/utils.cpp \
			src/fileUtils/hash.cpp

OBJ = $(SRC:.cpp=.o)

OUT = main

all: $(OUT)

$(OUT): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(OUT)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OUT) $(OBJ)

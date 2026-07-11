CXX = g++
CXXFLAGS = -std=c++23 -Iinclude -Wall
LDLIBS =

SRC = src/main.cpp \
      src/core/Commit.cpp \
      src/core/Branch.cpp \
      src/core/Repository.cpp \
      src/fileUtils/utils.cpp \
      src/fileUtils/hash.cpp \
      src/fileUtils/serialization.cpp \
      src/fileUtils/diff.cpp

DIFF_OBJ = src/fileUtils/diff.o

OBJ = $(SRC:.cpp=.o)

OUT = mgit
TEST_DIFF = test_diff
TEST_MERGE = test_merge

all: $(OUT)

$(OUT): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(OUT) $(LDLIBS)

$(TEST_DIFF): tests/test_diff.cpp $(DIFF_OBJ)
	$(CXX) $(CXXFLAGS) tests/test_diff.cpp $(DIFF_OBJ) -o $(TEST_DIFF)

$(TEST_MERGE): tests/test_merge.cpp $(DIFF_OBJ)
	$(CXX) $(CXXFLAGS) tests/test_merge.cpp $(DIFF_OBJ) -o $(TEST_MERGE)

test: $(TEST_DIFF) $(TEST_MERGE)
	./$(TEST_DIFF)
	./$(TEST_MERGE)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OUT) $(TEST_DIFF) $(TEST_MERGE) $(OBJ) tests/*.o

.PHONY: all test clean

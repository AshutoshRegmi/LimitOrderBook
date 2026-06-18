CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Isrc
SRC = src/OrderBook.cpp src/MatchingEngine.cpp src/main.cpp
TEST_SRC = src/OrderBook.cpp src/MatchingEngine.cpp tests/test_matching.cpp
TARGET = lob
TEST_TARGET = test_matching

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

$(TEST_TARGET): $(TEST_SRC)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_SRC)

run: $(TARGET)
	./$(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET)

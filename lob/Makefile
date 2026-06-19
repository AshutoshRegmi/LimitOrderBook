CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Isrc
SRC = src/OrderBook.cpp src/MatchingEngine.cpp src/main.cpp
TEST_MATCHING_SRC = src/OrderBook.cpp src/MatchingEngine.cpp tests/test_matching.cpp
TEST_CANCEL_SRC = src/OrderBook.cpp src/MatchingEngine.cpp tests/test_cancel.cpp
TARGET = lob
TEST_MATCHING = test_matching
TEST_CANCEL = test_cancel

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

$(TEST_MATCHING): $(TEST_MATCHING_SRC)
	$(CXX) $(CXXFLAGS) -o $(TEST_MATCHING) $(TEST_MATCHING_SRC)

$(TEST_CANCEL): $(TEST_CANCEL_SRC)
	$(CXX) $(CXXFLAGS) -o $(TEST_CANCEL) $(TEST_CANCEL_SRC)

run: $(TARGET)
	./$(TARGET)

test: $(TEST_MATCHING) $(TEST_CANCEL)
	./$(TEST_MATCHING)
	./$(TEST_CANCEL)

clean:
	rm -f $(TARGET) $(TEST_MATCHING) $(TEST_CANCEL)

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Isrc
SRC = src/OrderBook.cpp src/MatchingEngine.cpp src/Exchange.cpp src/Benchmark.cpp src/main.cpp
TEST_MATCHING_SRC = src/OrderBook.cpp src/MatchingEngine.cpp tests/test_matching.cpp
TEST_CANCEL_SRC = src/OrderBook.cpp src/MatchingEngine.cpp tests/test_cancel.cpp
TEST_MULTI_SYMBOL_SRC = src/OrderBook.cpp src/MatchingEngine.cpp src/Exchange.cpp tests/test_multi_symbol.cpp
TARGET = lob
TEST_MATCHING = test_matching
TEST_CANCEL = test_cancel
TEST_MULTI_SYMBOL = test_multi_symbol

.PHONY: all clean run test benchmark

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

$(TEST_MATCHING): $(TEST_MATCHING_SRC)
	$(CXX) $(CXXFLAGS) -o $(TEST_MATCHING) $(TEST_MATCHING_SRC)

$(TEST_CANCEL): $(TEST_CANCEL_SRC)
	$(CXX) $(CXXFLAGS) -o $(TEST_CANCEL) $(TEST_CANCEL_SRC)

$(TEST_MULTI_SYMBOL): $(TEST_MULTI_SYMBOL_SRC)
	$(CXX) $(CXXFLAGS) -o $(TEST_MULTI_SYMBOL) $(TEST_MULTI_SYMBOL_SRC)

run: $(TARGET)
	./$(TARGET)

benchmark: $(TARGET)
	mkdir -p benchmarks
	./$(TARGET) --benchmark

test: $(TEST_MATCHING) $(TEST_CANCEL) $(TEST_MULTI_SYMBOL)
	./$(TEST_MATCHING)
	./$(TEST_CANCEL)
	./$(TEST_MULTI_SYMBOL)

clean:
	rm -f $(TARGET) $(TEST_MATCHING) $(TEST_CANCEL) $(TEST_MULTI_SYMBOL)

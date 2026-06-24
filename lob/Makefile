CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Isrc
SRC = src/OrderBook.cpp src/MatchingEngine.cpp src/Exchange.cpp src/ITCHParser.cpp src/Benchmark.cpp src/main.cpp
TEST_MATCHING_SRC = src/OrderBook.cpp src/MatchingEngine.cpp tests/test_matching.cpp
TEST_CANCEL_SRC = src/OrderBook.cpp src/MatchingEngine.cpp tests/test_cancel.cpp
TEST_MULTI_SYMBOL_SRC = src/OrderBook.cpp src/MatchingEngine.cpp src/Exchange.cpp tests/test_multi_symbol.cpp
TEST_ITCH_SRC = src/OrderBook.cpp src/MatchingEngine.cpp src/Exchange.cpp src/ITCHParser.cpp tests/test_itch_parser.cpp
TARGET = lob
TEST_MATCHING = test_matching
TEST_CANCEL = test_cancel
TEST_MULTI_SYMBOL = test_multi_symbol
TEST_ITCH = test_itch_parser

.PHONY: all clean run test benchmark replay

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

$(TEST_MATCHING): $(TEST_MATCHING_SRC)
	$(CXX) $(CXXFLAGS) -o $(TEST_MATCHING) $(TEST_MATCHING_SRC)

$(TEST_CANCEL): $(TEST_CANCEL_SRC)
	$(CXX) $(CXXFLAGS) -o $(TEST_CANCEL) $(TEST_CANCEL_SRC)

$(TEST_MULTI_SYMBOL): $(TEST_MULTI_SYMBOL_SRC)
	$(CXX) $(CXXFLAGS) -o $(TEST_MULTI_SYMBOL) $(TEST_MULTI_SYMBOL_SRC)

$(TEST_ITCH): $(TEST_ITCH_SRC)
	@mkdir -p data
	$(CXX) $(CXXFLAGS) -o $(TEST_ITCH) $(TEST_ITCH_SRC)

run: $(TARGET)
	./$(TARGET)

benchmark: $(TARGET)
	mkdir -p benchmarks
	./$(TARGET) --benchmark

replay: $(TARGET)
	@mkdir -p data
	./$(TEST_ITCH) >/dev/null
	./$(TARGET) --replay data/sample_itch.bin

test: $(TEST_MATCHING) $(TEST_CANCEL) $(TEST_MULTI_SYMBOL) $(TEST_ITCH)
	./$(TEST_MATCHING)
	./$(TEST_CANCEL)
	./$(TEST_MULTI_SYMBOL)
	@mkdir -p data
	./$(TEST_ITCH)

clean:
	rm -f $(TARGET) $(TEST_MATCHING) $(TEST_CANCEL) $(TEST_MULTI_SYMBOL) $(TEST_ITCH)

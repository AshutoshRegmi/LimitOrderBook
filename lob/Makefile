CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
SRC = src/OrderBook.cpp src/main.cpp
TARGET = lob

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

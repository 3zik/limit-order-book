CXX      = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -O2

orderbook: OrderBook.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f orderbook

CXX      = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -O2

SOURCES = main.cpp Orderbook.cpp

orderbook: $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $@

.PHONY: clean
clean:
	rm -f orderbook

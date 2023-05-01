SOURCES := $(wildcard *.cpp)
EXECUTABLES := $(basename $(SOURCES))

CC := g++
CFLAGS := -O3 -Wall -Wextra -Wpedantic -std=c++14
DEBUGFLAGS := -g -Wall -Wextra -Wpedantic -std=c++14

all: $(EXECUTABLES)

debug: CFLAGS := $(DEBUGFLAGS)
debug: $(EXECUTABLES)

$(EXECUTABLES) : % : %.cpp $(SOURCES)
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	rm $(EXECUTABLES)

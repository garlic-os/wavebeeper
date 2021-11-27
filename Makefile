SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:.cpp=.obj)
EXECUTABLES := $(SOURCES:.cpp=.exe)

CC := cl
CFLAGS := /EHsc /MD /O2 /nologo

all: $(EXECUTABLES)

%.exe: %.cpp
	$(CC) $(CFLAGS) $<

.PHONY: clean

clean:
	del $(OBJECTS) $(EXECUTABLES)

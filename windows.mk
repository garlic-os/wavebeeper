SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:.cpp=.obj)
EXECUTABLES := $(SOURCES:.cpp=.exe)
ILKS := $(SOURCES:.cpp=.ilk)
SYMBOLS := $(SOURCES:.cpp=.pdb)

CC := cl
CFLAGS := /EHsc /MD /O2 /nologo
DEBUGFLAGS := /EHsc /MD /Zi /nologo

all: $(EXECUTABLES)

debug: CFLAGS = $(DEBUGFLAGS)
debug: $(EXECUTABLES)

%.exe: %.cpp
	$(CC) $(CFLAGS) $<

.PHONY: clean debug

clean:
	del /Q $(OBJECTS) $(EXECUTABLES) $(ILKS) $(SYMBOLS)

SOURCES := $(wildcard *.cpp)
OBJECTS     := $(ADDPREFIX out/, $(SOURCES:.cpp=.obj))
EXECUTABLES := $(ADDPREFIX out/, $(SOURCES:.cpp=.exe))
ILKS        := $(ADDPREFIX out/, $(SOURCES:.cpp=.ilk))
SYMBOLS     := $(ADDPREFIX out/, $(SOURCES:.cpp=.pdb))

CC := cl
CFLAGS := /EHsc /MD /O2 /Zi /nologo
DEBUGFLAGS := /EHsc /MD /Zi /nologo

all: $(EXECUTABLES)

debug: CFLAGS = $(DEBUGFLAGS)
debug: $(EXECUTABLES)

%.exe: %.cpp
	$(CC) $(CFLAGS) $<

.PHONY: clean debug

clean:
	del /Q $(OBJECTS) $(EXECUTABLES) $(ILKS) $(SYMBOLS)

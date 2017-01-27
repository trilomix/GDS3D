# Flags
CC=g++
CFLAGS=-c -w -O1 -I ../math/ -I ../gdsoglviewer/ -I ../libgdsto3d/
LDFLAGS=-L/usr/X11R6/lib64/ -lX11 -lGL -static-libgcc -static-libstdc++ 
# Static linking of stdc++ available starting at GCC 4.5

# Complicated system to fix .hash section, shame on you binutils guys!
LDD_MAJOR_VERSION_GTE3 := $(shell expr 2 \< `ldd --version | grep ldd | cut -d' ' -f4 | cut -d. -f1`)
LDD_MINOR_VERSION_GTE5 := $(shell expr 4 \< `ldd --version | grep ldd | cut -d' ' -f4 | cut -d. -f2`)
ifeq "$(LDD_MINOR_VERSION_GTE5)" "1"
	LDFLAGS += -Wl,--hash-style=both
else
	ifeq "$(LDD_MAJOR_VERSION_GTE3)" "1"
		LDFLAGS += -Wl,--hash-style=both
	endif
endif

# Include all sources in the folders
SOURCES=$(wildcard *.cpp) $(wildcard ../math/*.cpp) $(wildcard ../gdsoglviewer/*.cpp) $(wildcard ../libgdsto3d/*.cpp) 
HEADERS=$(wildcard *.h) $(wildcard ../math/*.h) $(wildcard ../gdsoglviewer/*.h) $(wildcard ../libgdsto3d/*.h) 

OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=./GDS3D

all: $(SOURCES) $(HEADERS) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean: # Clean object files
	rm -f $(OBJECTS) 

cleanall: # Also clean GDS3D executable
	rm -f $(OBJECTS) $(EXECUTABLE)

bininfo: # Information about the GDS3D binary
	@echo
	file $(EXECUTABLE) # Executable type
	@echo
	ldd $(EXECUTABLE) # Linked libraries
	@echo
	objdump -t $(EXECUTABLE) | grep @@ # External symbols
	@echo
	readelf -d $(EXECUTABLE) | grep NEEDED # Needed shared libraries
	@echo

sysinfo: # Information about the system
	@echo
	cat /etc/*-release # Linux distribution
	@echo
	g++ --version # GCC version
	@echo
	ldd --version # System libc version	
	@echo
	/sbin/ldconfig -p | grep libX11.so # Path to X11 library
	@echo
	/sbin/ldconfig -p | grep libGL.so # Path to OpenGL library
	@echo
	
glinfo: # Information about OpenGL on this system
	@echo
	glxinfo | grep direct # Direct rendering?
	@echo
	glxinfo | grep -i opengl # GL Version
	@echo
	glxinfo | grep -i 'GLX version' # GLX Version
	@echo
	glxinfo -l | grep GL_MAX | grep -v GL_MAX_PROGRAM # Limits
	@echo

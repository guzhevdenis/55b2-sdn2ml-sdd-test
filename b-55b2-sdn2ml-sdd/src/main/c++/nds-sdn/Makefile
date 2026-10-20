#+======================================================================
# NDS-SDN publisher (copied from 55b2-vnc-sdd nds3-sdn)
#-======================================================================

LIBNAME=nds3-sdn-publisher

LIBRARIES=nds3 rt sdn sdn-core ccs-core pthread log
LIBRARY_DIRS= $(CODAC_ROOT)/lib
INCLUDE_DIRS= ./include $(CODAC_ROOT)/include /usr/include/libxml2

TARGET=.
LIBRARY_DIR=$(TARGET)/lib
OBJECT_DIR=$(SOURCE_DIR)/.obj
SOURCE_DIR=.

SHAREDLIBRARY=$(LIBRARY_DIR)/lib$(LIBNAME).so
STATICLIBRARY=$(LIBRARY_DIR)/lib$(LIBNAME).a
INCLUDES=$(foreach inc,$(INCLUDE_DIRS),-I$(inc))
LDLIBS=$(foreach libs,$(LIBRARY_DIRS),-L$(libs) -Wl,-rpath,$(libs)) $(foreach libs,$(LIBRARIES),-l$(libs))
SOURCES=$(wildcard $(SOURCE_DIR)/*.cpp)

OBJECTS=$(addprefix $(OBJECT_DIR)/,$(patsubst %.cpp,%.o,$(notdir $(SOURCES))))

C=gcc
CC=g++

STD=-std=c++11
DEFS=-DNDS3_DLL -DNDS3_DLL_EXPORTS
CFLAGS=-c -Wall -Wextra -fPIC
CXXFLAGS=-c $(STD) -Wall -Wextra -fPIC -pthread -fvisibility-inlines-hidden $(DEFS)
LDFLAGS= -shared

ifeq ($(COVERAGE),true)
  CFLAGS+= -O0 -g --coverage -DDEBUG
  CXXFLAGS+= -O0 -g --coverage -DDEBUG
  LDFLAGS+= --coverage
else
  CFLAGS+= -O2
  CXXFLAGS+= -O2
endif

.PHONY: all clean run

all: $(SOURCES) $(SHAREDLIBRARY) $(STATICLIBRARY)

clean:
	rm -rf "$(SHAREDLIBRARY)" "$(STATICLIBRARY)" "$(OBJECT_DIR)"

run: $(SOURCES) $(SHAREDLIBRARY)
	$(SHAREDLIBRARY)

$(SHAREDLIBRARY): $(OBJECTS)
	mkdir -p $(LIBRARY_DIR)
	$(CC) $(LDFLAGS) $(LDLIBS) $(OBJECTS) -o $(SHAREDLIBRARY)

$(STATICLIBRARY): $(OBJECTS)
	mkdir -p $(LIBRARY_DIR)
	$(AR) rcs $@ $^

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(OBJECT_DIR)
	$(CC) $(CXXFLAGS) $(INCLUDES) $< -o $@

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c $(OBJECT_DIR)
	$(C) $(CFLAGS) $(INCLUDES) $< -o $@

$(OBJECT_DIR):
	mkdir -p $(OBJECT_DIR)

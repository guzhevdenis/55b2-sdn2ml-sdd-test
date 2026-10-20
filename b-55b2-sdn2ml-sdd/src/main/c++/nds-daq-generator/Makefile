#+======================================================================
# SIMULATOR NDS driver nds-daq-generator
#-======================================================================

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -fPIC -pthread -DNDS3_DLL -I/opt/codac/include -I./include/ -g
LDFLAGS = -L/opt/codac/lib
TARGETDIR = ./target/lib
LINK = -shared -fPIC -Wl,-as-needed -Wl,-rpath=.
TARGET = libnds3-NdsDaqGenerator.so
LIBS = -lnds3 -ldl -lpthread
SRCS = ./src/NdsDaqGenerator.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGETDIR)/$(TARGET)

$(TARGETDIR)/$(TARGET): $(OBJS)
	@mkdir -p $(TARGETDIR)
	$(CXX) $(LINK) $(LDFLAGS) -o $@ $^ $(LIBS)

.PHONY: clean
clean:
	$(RM) $(OBJS) $(TARGETDIR)/$(TARGET)

# Picked up automatically by ../Makefile; follows the existing CODAC_ROOT layout.
CXX ?= g++
CODAC_ROOT ?= /opt/codac
PREFIX ?= $(CODAC_ROOT)
TOPIC_DIR ?= /etc/opt/codac/sdn
CPPFLAGS += -I$(CODAC_ROOT)/include -I/usr/include/libxml2
CXXFLAGS += -std=c++11 -O2 -Wall -Wextra -pthread
LDFLAGS += -L$(CODAC_ROOT)/lib -Wl,-rpath,$(CODAC_ROOT)/lib
LDLIBS += -lsdn-core -lccs-core -lxml2 -llog -lrt -lpthread
SOURCES = main.cpp SdnTransport.cpp Processing.cpp
OBJECTS = $(SOURCES:%.cpp=.obj/%.o)
TOPICS = $(wildcard ../../resources/pcf*-nds-sdn-topic.xml) ../../resources/vnc-mp1-topic.xml

.PHONY: all clean install test
all: bin/vnc-mp1

bin/vnc-mp1: $(OBJECTS)
	mkdir -p bin
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

.obj/%.o: %.cpp
	mkdir -p .obj
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

bin/processing-test: tests/ProcessingTest.cpp Processing.cpp Processing.h
	mkdir -p bin
	$(CXX) $(CXXFLAGS) -I. tests/ProcessingTest.cpp Processing.cpp -o $@

test: bin/processing-test
	./bin/processing-test

install: all
	install -d $(DESTDIR)$(PREFIX)/bin $(DESTDIR)$(TOPIC_DIR)
	install -m 755 bin/vnc-mp1 $(DESTDIR)$(PREFIX)/bin/
	install -m 644 $(TOPICS) $(DESTDIR)$(TOPIC_DIR)/

clean:
	$(RM) $(OBJECTS) $(OBJECTS:.o=.d) bin/vnc-mp1 bin/processing-test

-include $(OBJECTS:.o=.d)

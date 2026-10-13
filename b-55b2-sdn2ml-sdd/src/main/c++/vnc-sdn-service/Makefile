# Picked up automatically by ../Makefile; follows the existing CODAC_ROOT layout.
.DEFAULT_GOAL := all
BUILD_ROOT ?= $(abspath ../../../../target)
PROGRAM = $(BUILD_ROOT)/bin/vnc-sdn-service
OBJECT_DIR = $(BUILD_ROOT)/obj/vnc-sdn-service

CXX ?= g++
CODAC_ROOT ?= /opt/codac
PREFIX ?= $(CODAC_ROOT)
TOPIC_DIR ?= /etc/opt/codac/sdn
CPPFLAGS += -I$(CODAC_ROOT)/include -I/usr/include/libxml2
CXXFLAGS += -std=c++11 -O0 -g -Wall -Wextra -pthread
LDFLAGS += -L$(CODAC_ROOT)/lib -Wl,-rpath,$(CODAC_ROOT)/lib
LDLIBS += -lsdn-core -lccs-core -lxml2 -llog -lrt -lpthread
SOURCES = vnc-sdn-service.cpp VncSdnPubSub.cpp VncDataProcessing.cpp
OBJECTS = $(addprefix $(OBJECT_DIR)/,$(SOURCES:.cpp=.o))
TOPICS = $(wildcard ../../resources/pcf*-nds-sdn-topic.xml) ../../resources/vnc-mp1-topic.xml

.PHONY: all clean install test
all: $(PROGRAM)

$(PROGRAM): $(OBJECTS) Makefile
	mkdir -p "$(@D)"
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o "$@"

$(OBJECT_DIR)/%.o: %.cpp Makefile
	mkdir -p "$(@D)"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c "$<" -o "$@"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

#bin/processing-test: tests/ProcessingTest.cpp Processing.cpp Processing.h
#	mkdir -p bin
#	$(CXX) $(CXXFLAGS) -I. tests/ProcessingTest.cpp Processing.cpp -o $@

#test: bin/processing-test
#	./bin/processing-test

#install: all
#	install -d $(DESTDIR)$(PREFIX)/bin $(DESTDIR)$(TOPIC_DIR)
#	install -m 755 bin/vnc-mp1 $(DESTDIR)$(PREFIX)/bin/
#	install -m 644 $(TOPICS) $(DESTDIR)$(TOPIC_DIR)/

clean:
	$(RM) $(OBJECTS) $(OBJECTS:.o=.d) bin/vnc-sdn-service
#bin/processing-test

-include $(OBJECTS:.o=.d)

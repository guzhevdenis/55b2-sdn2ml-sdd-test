# Both src/main/c++/vnc-mp1 and target/main/c++/vnc-mp1 are
# four levels below the unit root. Maven builds in the latter directory.
.DEFAULT_GOAL := all
BUILD_ROOT ?= $(abspath ../../../../target)
PROGRAM = $(BUILD_ROOT)/bin/vnc-mp1
OBJECT_DIR = $(BUILD_ROOT)/obj/vnc-mp1
TEST_PROGRAM = $(BUILD_ROOT)/test/bin/vnc-mp1-processing-test

CXX ?= g++
CODAC_ROOT ?= /opt/codac
PREFIX ?= $(CODAC_ROOT)
TOPIC_DIR ?= /etc/opt/codac/sdn
CPPFLAGS += -I$(CODAC_ROOT)/include -I/usr/include/libxml2
CXXFLAGS += -std=c++11 -O2 -Wall -Wextra -pthread
LDFLAGS += -L$(CODAC_ROOT)/lib -Wl,-rpath,$(CODAC_ROOT)/lib
LDLIBS += -lsdn-core -lccs-core -lxml2 -llog -lrt -lpthread
SOURCES = main.cpp SdnTransport.cpp Processing.cpp
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

$(TEST_PROGRAM): tests/ProcessingTest.cpp Processing.cpp Processing.h Makefile
	mkdir -p "$(@D)"
	$(CXX) $(CXXFLAGS) -I. tests/ProcessingTest.cpp Processing.cpp -o "$@"

test: $(TEST_PROGRAM)
	"$(TEST_PROGRAM)"

install: all
	install -d $(DESTDIR)$(PREFIX)/bin $(DESTDIR)$(TOPIC_DIR)
	install -m 755 "$(PROGRAM)" $(DESTDIR)$(PREFIX)/bin/
	install -m 644 $(TOPICS) $(DESTDIR)$(TOPIC_DIR)/

clean:
	$(RM) $(OBJECTS) $(OBJECTS:.o=.d) "$(PROGRAM)" "$(TEST_PROGRAM)"

-include $(OBJECTS:.o=.d)

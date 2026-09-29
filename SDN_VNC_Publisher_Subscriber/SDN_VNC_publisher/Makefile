CC = g++
CFLAGS = -Wall -std=c++11
INCLUDES = -I${CODAC_ROOT}/include -I/include -I/usr/include/libxml2 -I.
LDFLAGS = -L/lib -L${CODAC_ROOT}/lib -L/usr/lib64
LIBS = -lpthread -lrt -lsdn -lsdn-core -lccs-core -llog -lxml2

SRCS = SDN_VNC_publisher.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = SDN_VNC_publisher

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) $(OBJS) $(TARGET)

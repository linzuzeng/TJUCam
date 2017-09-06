CC = g++
CFLAGS = -g -Wall
SRCS = CenterPaho.cpp
PROG = CenterPaho

OPENCV = `pkg-config opencv --cflags --libs`
LIBS = $(OPENCV) -lpaho-mqtt3a

all:
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)
	mv $(PROG) ../
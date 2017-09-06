CC = g++
CFLAGS = -g -Wall


OPENCV = `pkg-config opencv --cflags --libs`
EIGEN = `pkg-config eigen3 --cflags --libs`
LIBS = $(OPENCV)  $(EIGEN) -lpaho-mqtt3a -lpaho-mqtt3c

all:calib1 calib2 calib3 calib4
calib1:jiaozhun1.cpp
	$(CC) $(CFLAGS) -o calib1 jiaozhun1.cpp $(LIBS)
calib2:jiaozhun2.cpp
	$(CC) $(CFLAGS) -o calib2 jiaozhun2.cpp $(LIBS)
calib3:jiaozhun3.cpp
	$(CC) $(CFLAGS) -o calib3 jiaozhun3.cpp $(LIBS)
calib4:jiaozhun4.cpp
	$(CC) $(CFLAGS) -o calib4  jiaozhun4.cpp $(LIBS)
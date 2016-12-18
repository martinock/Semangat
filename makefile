all: 
	make bin/transmitter bin/receiver

bin/transmitter: src/transmitter.cpp src/transmitwindow.cpp src/transmitwindow.h src/dcomm.h
	g++ -pthread -o bin/transmitter src/transmitter.cpp src/transmitwindow.cpp

bin/receiver: src/receiver.cpp src/receivewindow.h src/receivewindow.cpp src/dcomm.h
	g++ -pthread -o bin/receiver src/receiver.cpp src/receivewindow.cpp

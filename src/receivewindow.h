#ifndef receivewindow_h
#define receivewindow_h


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>  
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <sys/wait.h>
#include <bitset>
#include <cstring>
#include "dcomm.h"

#define MAXRECVBUFF 10 

using namespace std;

typedef struct ReceiveWindow
{
	unsigned int front;
	unsigned int rear;
	unsigned int maxsize;
	Byte *data;
	bool *received; // apakah frame tersebut telah diterima atau tidak
} ReceiveWindow;


void kirimACK(Byte ack, int sockfd, struct sockaddr_in sender_addr, 
	int slen, unsigned int msgno, int checksum);


string buatCRC(string BitString);
string getBitString(MESGB message);
bool isCheckSumCorrect(MESGB message);
bool isFrameValid(MESGB message);

void perlebarwindow(ReceiveWindow* window);
void persempitwindow(ReceiveWindow* window);
void masukkankebuff(Byte data, QTYPE *queue, int sockfd, struct sockaddr_in sender_addr, int slen);

#endif

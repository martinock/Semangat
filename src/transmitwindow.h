#ifndef transmitwindow_h
#define transmitwindow_h

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <sys/wait.h>
#include <string.h>
#include <cstring>
#include <bitset>
#include <time.h>
#include "dcomm.h"

using namespace std;

typedef struct TransmitWindow
{
	unsigned int count;
	unsigned int front;
	unsigned int rear;
	unsigned int maxsize;
	Byte *data;
	bool *ack; //apakah frame yang dikirimkan sudah diack atau belum
	clock_t *startTime;
} TransmitWindow;

//Mengirimkan Pesan
void kirimpesan(Byte msgno, Byte data, int socket_desc, struct sockaddr_in server, int slen);

//Membuat CRC dari bitstring
string buatCRC(string BitString);

//Membuat BitString dari message
string getBitString(MESGB message);

//Generate checksum dari message untuk dikirimkan
Byte getCheckSum(MESGB message);

//Menambahkan byte data ke bagian belakang window
void tambahkebelakang(Byte data, TransmitWindow *window);

//Menghapus data terdepan window
void hapusdepan(TransmitWindow *window);

#endif

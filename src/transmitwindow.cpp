#include "transmitwindow.h"

using namespace std;

void kirimpesan(Byte msgno, Byte data, int socket_desc, struct sockaddr_in server, int slen){
    MESGB message;
    message.soh = SOH;
    message.msgno = msgno;
    message.stx = STX;
    message.data = data;
    message.etx = ETX;

    message.checksum = getCheckSum(message);

    sendto(socket_desc, &message, sizeof(MESGB), 0, 
                    (struct sockaddr*)&server, slen);
}

string buatCRC(string BitString)
{
   static char Res[8];                                 
   char CRC[7];
   int  i;
   char DoInvert;
   
   for (i=0; i<7; ++i){
	   CRC[i] = 0;
	}
   
   for (i=0; i<BitString.length(); ++i)
      {
      DoInvert = ('1'==BitString[i]) ^ CRC[6];         

      CRC[6] = CRC[5] ^ DoInvert;
      CRC[5] = CRC[4];
      CRC[4] = CRC[3] ^ DoInvert;
      CRC[3] = CRC[2];
      CRC[2] = CRC[1] ^ DoInvert;
      CRC[1] = CRC[0];
      CRC[0] = DoInvert;
      }
      
   for (i=0; i<7; ++i)  Res[6-i] = CRC[i] ? '1' : '0'; // Convert binary to ASCII
   Res[7] = 0;                                         // Set string terminator

   return(Res);
}

string getBitString(MESGB message){
    string BitString = "";
    BitString += bitset<8>(message.soh).to_string();
    BitString += bitset<8>(message.msgno).to_string();
    BitString += bitset<8>(message.stx).to_string();
    BitString += bitset<8>(message.data).to_string();
    BitString += bitset<8>(message.etx).to_string();

    return BitString;
}

Byte getCheckSum(MESGB message){
    string BitString = getBitString(message);
    string BitCheckSum = buatCRC(BitString);

    Byte checksum = (Byte) (bitset<8>(BitCheckSum).to_ulong());

    return checksum;
}

void tambahkebelakang(Byte data, TransmitWindow *window){
    unsigned int rear = window->rear;
    window->data[rear] = data;
    window->ack[rear] = false;
    window->startTime[rear] = -1;
    window->rear = (rear + 1) % window->maxsize;
    window->count++;
}

void hapusdepan(TransmitWindow *window){
    window->front = (window->front + 1) % window->maxsize;
    window->count--;
}

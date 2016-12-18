#include "receivewindow.h"

using namespace std;

void kirimACK(Byte ack, int sockfd, struct sockaddr_in sender_addr, int slen, 
	unsigned int msgno, int checksum)
{
	RESP response;
    response.ack = ack;
    response.msgno = msgno;
    response.checksum = checksum;

	if(sendto(sockfd, &response, sizeof(RESP), 0, 
     	(struct sockaddr *) &sender_addr, slen) == -1)
	{	
		cout<<"Eror mengirimkan respon"<<endl;
    }
}


string buatCRC(string BitString)
{
   
   static char Res[8];                                
   char CRC[7];
   int  i;
   char DoInvert;
   
   for (i=0; i<7; ++i)  CRC[i] = 0;                   
   
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
      
   for (i=0; i<7; ++i)  Res[6-i] = CRC[i] ? '1' : '0'; 
   Res[7] = 0;                                         

   return(Res);
}

string getBitString(MESGB message){
    string BitString = "";
    BitString += bitset<8>(message.soh).to_string();
    BitString += bitset<8>(message.msgno).to_string();
    BitString += bitset<8>(message.stx).to_string();
    BitString += bitset<8>(message.data).to_string();
    BitString += bitset<8>(message.etx).to_string();
    BitString += bitset<7>(message.checksum).to_string();

    return BitString;
}

bool isCheckSumCorrect(MESGB message){
    string BitString = getBitString(message);
    string BitCheckSum = buatCRC(BitString);

    return (bitset<7>(BitCheckSum).to_ulong()) == 0;
}

bool isFrameValid(MESGB message){
    return (message.soh == SOH && message.msgno < MAXRECVBUFF && 
            message.stx == STX && message.etx == ETX && isCheckSumCorrect(message));
}

void perlebarwindow(ReceiveWindow* window){
    window->received[window->rear] = false;
    window->rear = (window->rear + 1) % window->maxsize;
}

void persempitwindow(ReceiveWindow* window){
    window->front = (window->front + 1) % window->maxsize;
}

void masukkankebuff(Byte data, QTYPE *queue, int sockfd, struct sockaddr_in sender_addr, int slen){
    queue->data[queue->rear++] = data;
    queue->rear %= queue->maxsize;
    queue->count++;
}

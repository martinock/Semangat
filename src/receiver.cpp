#include "receivewindow.h"

using namespace std;

#define RXQSIZE 8

Byte rxbuf[RXQSIZE];
QTYPE rcvq = { 0, 0, 0, RXQSIZE, rxbuf };
QTYPE *rxq = &rcvq;
bool send_xoff = false;

char yangditerima[100];
int zzz = 0;


// Untuk socket
int socket_desc;
int bytedikonsumsi = 1;
struct sockaddr_in server, client;
int slen = sizeof(client);
int portNo;


// Untuk Window
Byte buf[MAXRECVBUFF];
bool received[MAXRECVBUFF];
ReceiveWindow ww = {0, MAXRECVBUFF/2 - 1, MAXRECVBUFF, buf, received};


static Byte rcvchar(int socket_desc, QTYPE *queue);
static Byte q_get(QTYPE *);
void* consume(void*);


int main(int argc, char *argv[]) {

	Byte c;
	
	// Menciptakan Socket
	socket_desc = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(socket_desc < 0){
		printf("Tidak dapat membentuk socket\n");
		return 1;
	}
	
	printf("Socket telah terbentuk\n");
	memset((char *)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(atoi(argv[1]));
	
	//Melakukan Bind
	
	if(bind(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0){
		perror("Bind gagal\n");
		return 1;
	}
	printf("%s %s%s%d\n", "Binding pada", inet_ntoa(server.sin_addr), ":", atoi(argv[1]));
	

// Membuat thread
	pthread_t t1;
	if(pthread_create(&t1, NULL, &consume, NULL) < 0){
		fprintf(stderr, "Error menciptakan thread\n");
		return 1;
	}

	MESGB message;
    while(true){
	    if(recvfrom(socket_desc, &message, sizeof(MESGB), 0, 
				(struct sockaddr *) &client, (socklen_t *) &slen) == -1)
		{     
	        cout<<"Eror menerima byte"<<endl;
	        exit(EXIT_FAILURE);
	    }

    	// Send response back
	    if(isFrameValid(message) && rcvq.count < RXQSIZE){
	    	int msgno = message.msgno;
	    	Byte data = message.data;
	    	ww.data[msgno] = data;

	    	kirimACK(ACK, socket_desc, client, slen, msgno, message.checksum);
	    	if(data != Endfile){
		    	printf("Frame nomor : %d diterima (Byte diterima adalah : %c)\n", msgno, data);
		    	ww.received[msgno] = true;
	    	}
		    else
		    	break;
	    }
	    else if(rcvq.count >= RXQSIZE){
	    	cout<<"Buffer telah penuh , tidak mengirikan ACK"<<endl;
	    }
	    else{
	    	cout<<"Gagal : Salah Checksum. Mengirimkan NAK"<<endl;
	    	kirimACK(NAK, socket_desc, client, slen, message.msgno, message.checksum);
	    }

	    for(int i = ww.front; i != ww.rear; i = (i+1)%ww.maxsize){
	    	if(i == ww.front && ww.received[i]){
	    		masukkankebuff(ww.data[i], rxq, socket_desc, client, slen);
	    		persempitwindow(&ww);
	    		perlebarwindow(&ww);
	    	}
	    }
	}
	while(rxq->count!=0){
	}
	
	cout<<"Yang diterima adalah : "<<yangditerima<<endl;
	
	return 0;
}


static Byte q_get(QTYPE *queue) {
	Byte *ch;
	
	if (!queue->count) {
		return (NULL);
	}
	
	Byte c = queue->data[queue->front++];
	queue->front %= queue->maxsize;
	queue->count--;
	
	if(c != Endfile){
		printf("Konsumsi byte ke %d : '%c'\n",bytedikonsumsi, c);
		yangditerima[zzz]=c;
		zzz++;
		bytedikonsumsi++;
	}
	
	return c;
}

void* consume(void*){
	//Mengonsumsi buffer
	
	int i = 1;
	while (true) {
		q_get(rxq);
		usleep(200000); //delay
	}
	pthread_exit(0);
	
}

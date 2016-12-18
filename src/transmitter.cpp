#include "transmitwindow.h"

using namespace std;

#define MAXTRANSBUFF 10
#define TIMEOUT 1000 // in ms

struct sockaddr_in serv_addr;
int socket_desc;
int slen = sizeof(serv_addr);
static Byte lastReceivedChar;
FILE *fp;
Byte buf[MAXTRANSBUFF];
bool ack[MAXTRANSBUFF];
clock_t startTime[MAXTRANSBUFF];
TransmitWindow ww = {0,0,0, MAXTRANSBUFF, buf, ack, startTime};

void* sendSignal(void*);
void* receiveMessage(void*);

int main(int argc, char* argv[]){

// Menciptakan Socket
	if(argc < 4){
		fprintf(stderr,"Perintah salah. Kurang argumen\n");
        exit(0);
	}

// Membuka  socket
	socket_desc = socket(PF_INET, SOCK_DGRAM, 0);
	if(socket_desc < 0){
		perror("Socket error\n");
        exit(1);
	}
	
	
// Mengosongkan buffer 
    bzero((char *) &serv_addr, sizeof(serv_addr));
	
// Menginisialisasi address penerima/receiver
	serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    printf("Membuat socket untuk koneksi ke %s:%s ...", argv[1], argv[2]);
    
	serv_addr.sin_port = htons(atoi(argv[2]));

	
// Membuat thread untuk menerima respon
	pthread_t send_signal_thread;
	
	if(pthread_create(&send_signal_thread, NULL, &sendSignal, NULL) < 0){
		fprintf(stderr, "Pembuatan Thread error\n");
  		return 1;
	}
	
	
	fp = fopen(argv[3] , "r");
	if(fp == NULL){
		perror("File error\n");
		exit(1);
	}
	
	Byte c;
	bool endfile = false;
	while(true){
		// Membaca pesan dan meletekkan ke window
		while((ww.count <= ww.maxsize / 2) && !endfile){
			if(lastReceivedChar != XOFF){
				if(fscanf(fp, "%c", &c) == EOF){
					c = Endfile; //Endfile sequence
					endfile = true;
				}
				tambahkebelakang(c, &ww);
			}
		}

		// Melakukan iterasi pada window dan mengirimkan frame yang belum di ACK
		for(int i = ww.front; i!=ww.rear; i = (i+1) % ww.maxsize){
			if(!ww.ack[i]){
				double selisihwaktu = (double)(clock() - ww.startTime[i])/CLOCKS_PER_SEC * 1000;
				if(ww.startTime[i]== -1 || selisihwaktu>TIMEOUT){
					if(ww.startTime[i] != -1)
						printf("Time out untuk frame nomor %d\n",i);

					ww.startTime[i] = clock();
					kirimpesan(i, ww.data[i], socket_desc, serv_addr, slen);
					printf("Mengirimkan message nomor -%d : '%c'\n", i, ww.data[i]);
				}
			}
			else if(i == ww.front && ww.ack[i]){
				//Melakukan slide
				hapusdepan(&ww);
			}
		}

		if(endfile && ww.front == ww.rear)
			break; // Seluruh pesan telah terkirim
	}

	fclose(fp);

	return 0;	
}

void* sendSignal(void*){
	
	
	RESP response;
	while(true){
		if(recvfrom(socket_desc, &response, sizeof(RESP), 0, 
				(struct sockaddr*)&serv_addr, (socklen_t*) &slen) == -1)
			{
				cout<<"Error receiving byte"<<endl;
				exit(EXIT_FAILURE);
			}

		if(response.ack == ACK){
			printf("ACK didapatkan dari message nomor: %d\n", response.msgno);
			ww.ack[response.msgno] = true;
		}
		else{
			printf("NAK didapatkan dari message nomor: %d\n", response.msgno);
			
			// Resendings
			int nomormessage = response.msgno;
			int data = ww.data[nomormessage];

			ww.startTime[nomormessage] = clock();
			kirimpesan(nomormessage, data, socket_desc, serv_addr, slen);
			printf("Mengirimkan message nomor -%d: '%c'\n", nomormessage, data);
		}
	}
}

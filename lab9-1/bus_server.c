//2021115737 hajaehyeon
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>

#define ROWS 2
#define COLS 10
#define MAX_CLNT 256

typedef struct{
    int command;
    int seatno;
    int seats[ROWS][COLS];
    int result;
}RES_PACKET;

typedef struct{
    int command;
    int seatno;
}REQ_PACKET;

RES_PACKET spacket;
REQ_PACKET qpacket;

void * handle_clnt(void * arg);
void error_handling(char * msg);

int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
  
	pthread_mutex_init(&mutx, NULL);
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");

    memset(&spacket, 0, sizeof(spacket));
    memset(&qpacket, 0, sizeof(qpacket));

    printf("------------------------\n");
    printf(" Bus Reservation Systen\n");
    printf("------------------------\n");

    while(1)
	{
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		
		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++]=clnt_sock;
		pthread_mutex_unlock(&mutx);
	
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		printf("Connected client IP: %s clnt_sock=%d\n", inet_ntoa(clnt_adr.sin_addr), clnt_sock);
	}

	close(serv_sock);
	return 0;
}

void * handle_clnt(void * arg)
{  
	int clnt_sock=*((int*)arg);
    int str_len = 0, i;
    int row, col, seat;

    while(1){
        read(clnt_sock, &qpacket, sizeof(qpacket));
        
        row = (qpacket.seatno - 1) / COLS;
        col = (qpacket.seatno - 1) % COLS; 
        seat = spacket.seats[row][col];
        spacket.command = qpacket.command;

        if(qpacket.command == 1){
            spacket.result = 0;
            write(clnt_sock, &spacket, sizeof(spacket));
        }
        else if(qpacket.command == 2){
            spacket.seatno = qpacket.seatno;
            pthread_mutex_lock(&mutx);
            if(qpacket.seatno>20){
                spacket.result = -1;
            }
            else if(seat!=0){
                spacket.result = -2;
            }
            else{
                spacket.result = 0;
                spacket.seats[row][col] = clnt_sock;
            }
            pthread_mutex_unlock(&mutx);
            write(clnt_sock, &spacket, sizeof(spacket));
        }
        else if(qpacket.command == 3){
            spacket.seatno = qpacket.seatno;
            if(seat==0){
                spacket.result = -3;
            }
            else if(seat!=clnt_sock){
                spacket.result = -4;
            }
            else{
                spacket.result = 0;
                spacket.seats[row][col] = 0;
            }
             write(clnt_sock, &spacket, sizeof(spacket));
        }
        else if(qpacket.command == 4){
            pthread_mutex_lock(&mutx);
            for(i=0; i<clnt_cnt; i++) 
	        {
		        if(clnt_sock==clnt_socks[i])
		        {
			        while(i < clnt_cnt)
			        {
				        clnt_socks[i]=clnt_socks[i+1];
				        i++; 
			        }
			        break;
		        }
	        }
	        clnt_cnt--;
            printf("Client removed: clnt_sock=%d, %d\n",clnt_sock,i);
            close(clnt_sock);
            pthread_mutex_unlock(&mutx);
            break;
        }
    }
}

void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

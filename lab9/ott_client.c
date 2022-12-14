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
#include <time.h>

#define BASIC_BUF 10
#define STANDARD_BUF 100
#define PREMIUM_BUF 1000

#define MAX_SIZE PREMIUM_BUF

#define FILE_REQ 0
#define FILE_SENDING 1
#define FILE_END 2
#define FILE_END_ACK 3
#define MAX_CLNT 256

typedef struct {
    int command;
    int type;
    char buf[MAX_SIZE];
    int len; 
} PACKET;

void * recv_msg(void * arg);
void error_handling(char * msg);

int clnt_cnt=0;
int clnt_socks[MAX_CLNT];

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
    pthread_t rcv_id;
    void * thread_return;
    PACKET packet;
    int menu;

	if(argc!=3) {
		printf("Usage : %s %s <port>\n", argv[0], argv[1]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");
	
    while(1){
        printf("-----------------------------------------------\n");
        printf("        K-OTT Service       \n");
        printf("-----------------------------------------------\n");
        printf(" Choose a subscribe type\n");
        printf("-----------------------------------------------\n");
        printf("1: Basic, 2: Standard, 3:Premeum, 4: quit: ");
        scanf("%d", &packet.type);
        if(packet.type == 4){
            printf("Exit program\n");
            close(sock);
            break;
        }
        printf("-----------------------------------------------\n");
        printf("1. Download, 2:Back to Main menu: ");
        scanf("%d", &menu);
        if(menu == 1){
            packet.command = FILE_REQ;
            write(sock, &packet, sizeof(packet));
            pthread_create(&rcv_id, NULL, recv_msg, (void*)&sock);
            pthread_join(rcv_id, NULL);
            close(sock);
            break;
        }
        if(menu == 2)
            continue;       
    }  
	return 0;
}

void * recv_msg(void * arg)   // read thread main
{
    int sock=*((int*)arg);
	int str_len, read_len  = 0;
    PACKET packet;

    unsigned long nano = 1000000000;
    unsigned long t1, t2;
    struct timespec start, end;

    memset(&packet, 0, sizeof(packet)); 

    clock_gettime(CLOCK_REALTIME, &start);
    t1 = start.tv_nsec + start.tv_sec *	nano;
	while(1)
	{   
        str_len=read(sock, &packet,sizeof(packet));
		if(str_len==-1) 
			return (void*)-1;

        read_len += packet.len;
		if(packet.command == FILE_SENDING){
            printf(".");
        }
		else if(packet.command == FILE_END){
            clock_gettime(CLOCK_REALTIME, &end);
            t2 = end.tv_nsec + end.tv_sec *	nano;
            
            printf("\nFile Transmission Finished\n");
            printf("Total received bytes: %d\n", read_len);
            printf("Downloading time: %ld msec\n", (t2-t1)/1000000);	
            printf("Client closed\n");

            packet.command = FILE_END_ACK;
            write(sock, &packet, sizeof(packet));
            close(sock);
            break;
        }
        memset(&packet, 0, sizeof(packet)); 
	}
	return NULL;
}
	
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

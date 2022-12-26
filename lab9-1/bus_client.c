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
#include <semaphore.h>

static sem_t sem_one;
static sem_t sem_two;

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

void * recv_msg(void * arg);
void * send_msg(void * arg);
void error_handling(char * msg);

int clnt_cnt=0;
int clnt_socks[MAX_CLNT];

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
    pthread_t send_id, rcv_id;
    void * thread_return;

	if(argc!=3) {
		printf("Usage : %s %s <port>\n", argv[0], argv[1]);
		exit(1);
	}
	
    sem_init(&sem_one, 0, 0);
    sem_init(&sem_two, 0, 1);

	sock=socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");

    pthread_create(&rcv_id, NULL, recv_msg, (void*)&sock);
    pthread_create(&send_id, NULL, send_msg, (void*)&sock);

    pthread_join(rcv_id, NULL);
    pthread_join(send_id, NULL);

    sem_destroy(&sem_one);
    sem_destroy(&sem_two);

    close(sock);
    return 0;
}

void * send_msg(void * arg)   
{   
    int sock=*((int*)arg);
    REQ_PACKET qpacket;

    while(1){
        sem_wait(&sem_two);
        printf("1: inquiry, 2: reservation, 3: cancellation, 4: quit: ");
        scanf("%d", &qpacket.command);
        if(qpacket.command == 2){
            printf("Input seat number: ");
            scanf("%d", &qpacket.seatno);
        }
        else if(qpacket.command == 3){
            printf("Input seat number for cancellation: ");
            scanf("%d", &qpacket.seatno);
        }
        write(sock, &qpacket, sizeof(qpacket));
        sem_post(&sem_one);
        if(qpacket.command==4){
            printf("Quit.\n");
            break;
        }
    }

    return NULL;
}

void * recv_msg(void * arg)   
{
    int sock=*((int*)arg);
	int str_len;
    RES_PACKET spacket;

    while(1){
        sem_wait(&sem_one);
        
        read(sock, &spacket, sizeof(spacket));

        printf("---------------------------------------------------\n");
        printf("               BUS Reservation System              \n");
        printf("---------------------------------------------------\n");
        printf("|  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 |\n");
        printf("---------------------------------------------------\n");
        for (int i = 0; i < COLS; i++) {
            printf("| %2d ", spacket.seats[0][i]);
        }
        printf("|\n");
        printf("---------------------------------------------------\n");
        printf("| 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 |\n");
        printf("---------------------------------------------------\n");
        for (int i = 0; i < COLS; i++) {
            printf("| %2d ", spacket.seats[1][i]);
        }
        printf("|\n");
        printf("---------------------------------------------------\n");

        if(spacket.result == 0){
            printf("Operation success.\n");
        }
        else if(spacket.result == -1){
            printf("Wrong seat number.\n");
        }
        else if(spacket.result == -2){
            printf("Reservation failed. (The seat was already reserved.)\n");
        }
        else if(spacket.result == -3){
            printf("Cancellation failed. (The seat was not reserved.)\n");
        }
        else if(spacket.result == -4){
            printf("Cancellation failed. (The seat was reserved by another person.)\n");
        }
        sem_post(&sem_two);
    }

    return NULL;
}

void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
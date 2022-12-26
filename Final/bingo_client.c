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

#define ROW 4
#define COL 4

static sem_t sem_one;

#define BINGO_READY		0 	// Server -> Client
#define BINGO_REQUEST 	1	// Client -> Server
#define BINGO_RESPONSE	2	// Server -> Client
#define BINGO_END		3	// Server -> Client 

#define FAIL   -1
#define SUCCESS 1

typedef struct {
	int cmd;			// cmd field value
	int board[ROW][COL]; // 숫자를 맞춘 클라이언트 id (4: client #1, 5: client #2)
	int result; 		// FAIL, SUCCESS
}RES_PACKET;


typedef struct {
	int cmd;
	int bingo_num;
}REQ_PACKET;

int end = 0;
int menu = 0;

void * recv_msg(void * arg);
void * send_msg(void * arg);
void error_handling(char * msg);

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
    
	sock=socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");

    printf("Choose a player type(1: Fast player, 2: Slow player): ");
    scanf("%d", &menu);

    pthread_create(&rcv_id, NULL, recv_msg, (void*)&sock);
    pthread_create(&send_id, NULL, send_msg, (void*)&sock);

    pthread_join(rcv_id, NULL);
    pthread_join(send_id, NULL);

    sem_destroy(&sem_one);

    close(sock);
    return 0;
}

void *send_msg(void * arg)   
{   
    int sock=*((int*)arg);
    REQ_PACKET qpacket;
    
    sem_wait(&sem_one);
    srand((unsigned int)time(NULL));
    while(!end){
        memset(&qpacket, 0, sizeof(qpacket));
        qpacket.cmd = BINGO_REQUEST;
        qpacket.bingo_num = rand() % (ROW * COL) + 1;
        write(sock, &qpacket, sizeof(qpacket));
        printf("[Tx] cmd: %d, bingo_num: %d, delay: %d\n", qpacket.cmd, qpacket.bingo_num, menu);
        sleep(menu);
    }

    return NULL;
}

void * recv_msg(void * arg)   
{
    int sock=*((int*)arg);
    int clnt_socks[256];
    RES_PACKET spacket;

    read(sock, &spacket, sizeof(spacket));
    printf("[Rx] cmd: %d, result %d\n", spacket.cmd, spacket.result);
    if(spacket.cmd == BINGO_READY){
        printf("BINGO_READY received\n");
        sem_post(&sem_one);
    }
            
    while(1){
        read(sock, &spacket, sizeof(spacket));
        read(sock, clnt_socks,sizeof(clnt_socks));
        printf("[Rx] cmd: %d, result: %d\n", spacket.cmd, spacket.result);

        if(spacket.cmd == BINGO_RESPONSE || spacket.cmd == BINGO_END){     
            printf("+-------------------+\n");
            for(int i=0; i < ROW; i++)
            {
                for(int j=0; j < COL; j++)
                {   
                    if(spacket.board[i][j] == 0){
                        printf("|    ");
                    }
                    else if(spacket.board[i][j] == clnt_socks[0]){
                        printf("|   O");
                    }
                    else if(spacket.board[i][j] == clnt_socks[1]){
                        printf("|   X");
                    }
                }
                printf("|\n");
                printf("+-------------------+\n");
            }
        }
        if(spacket.cmd == BINGO_END){
            end = 1;
            printf("BINGO_END. Game is over\n");
            int c1=0, c2=0;
            for(int i=0; i < ROW; i++)
            {
                for(int j=0; j < COL; j++)
                {
                    if(spacket.board[i][j] == clnt_socks[0]){
                        c1++;
                    }
                    else if(spacket.board[i][j] == clnt_socks[1]){
                        c2++;
                    }
                }
            }
            if(c1>c2){
                printf("Client1 win [client1(%d) : client2(%d)]\n",c1, c2);
            }
            else if(c1<c2){
                printf("Client2 win [client1(%d) : client2(%d)]\n",c1, c2);
            }
            else{
                printf("tie\n");
            }
            break;
        }
    }
    return NULL;
}

void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

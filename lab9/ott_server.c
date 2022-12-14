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
	
    printf("-----------------------\n");
    printf(" K-OTT Service Server\n");
    printf("-----------------------\n");

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
	int str_len = 0, write_len = 0, i;
	PACKET packet;
    char type[20];
    int num;

    memset(&packet, 0, sizeof(packet));
	
	read(clnt_sock, &packet, sizeof(packet));
	if(packet.command == FILE_REQ){
        if(packet.type==1){
            strcpy(type,"(Basic)");
            num = BASIC_BUF;
        }
        else if(packet.type==2){
            printf("ss");
            strcpy(type,"(Standard)");
            num = STANDARD_BUF;
        }
        else if(packet.type==3){
            strcpy(type,"(Premium)");   
            num = PREMIUM_BUF;
        }
        int fd = open("hw06.mp4",O_RDONLY);
        if(fd == -1)
            error_handling("open() error");

        while(str_len = read(fd, packet.buf, num)){
            if(str_len == -1)
                return (void*)-1;
            packet.len = str_len;
            write_len += packet.len;

            if(str_len == num){
                packet.command = FILE_SENDING;
                write(clnt_sock, &packet, sizeof(packet));
            }
            else if(str_len != num){
                close(fd);
                packet.command = FILE_END;
                write(clnt_sock, &packet, sizeof(packet));
                break;
            }    
            memset(&packet, 0, sizeof(packet)); 
        }

        str_len = read(clnt_sock, &packet, sizeof(packet));
        if(str_len == -1)
            return (void*)-1;
        if(packet.command == FILE_END_ACK){
            pthread_mutex_lock(&mutx);
            printf("\nTotal Tx Bytes: %d to Client %d %s\n",write_len, clnt_sock, type);
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
            printf("[Rx] FILE_END_ACK from Client %d => clnt_sock: %d closed.\n\n",clnt_sock,clnt_sock);
            close(clnt_sock);
            pthread_mutex_unlock(&mutx);
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

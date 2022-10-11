//2021115737 hajaehyeon
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

void error_handling(char *message);

typedef struct {
	int cmd; //	0:	request,	1:	response,	2:	quit
	char addr[20]; //	dotted-decimal	address 저장(20	bytes)
	struct in_addr iaddr;  //	inet_aton()	result 저장
	int result;	//	0:	Error,	1:	Success	
} PACKET;


int main(int argc, char* argv[])
{
	int sock;
    int read_len=0;
	
    struct sockaddr_in serv_addr;
    PACKET dotted;

	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1)
		error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
		
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1) 
		error_handling("connect() error!");
    
    while(1){
        printf("Input dotted-decimal address: ");
	    scanf("%s",dotted.addr);

        if(strcmp(dotted.addr, "quit") == 0){
            dotted.cmd = 2;
            write(sock,&dotted,sizeof(dotted));
            printf("[Tx] cmd: %d(QUIT)\n",dotted.cmd);
            break;
        }

        dotted.cmd = 0;
        write(sock,&dotted,sizeof(dotted));
        printf("[Tx] cmd: %d, addr: %s\n",dotted.cmd,dotted.addr);

        read_len = read(sock,&dotted,sizeof(dotted));
		if(read_len == -1)
			error_handling("read() error!");

        if(dotted.result == 0)
            printf("[Rx] cmd: %d, Address conversion fail! (result: %d)\n",dotted.cmd,dotted.result);
        else
            printf("[Rx] cmd: %d, addr: %#x (result: %d)\n",dotted.cmd,dotted.iaddr.s_addr,dotted.result);

        printf("\n");
    }

	close(sock);
    printf("Client socket close and exit.\n");
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

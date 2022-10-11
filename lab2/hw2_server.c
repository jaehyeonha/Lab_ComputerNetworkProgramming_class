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


int main(int argc, char *argv[])
{
	int serv_sock;
	int clnt_sock;
	int read_len=0;

	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	PACKET dotted;
	socklen_t clnt_addr_size;
	
	if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1)
		error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
		error_handling("bind() error"); 
	
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	clnt_addr_size=sizeof(clnt_addr);  
	clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
	if(clnt_sock==-1)
		error_handling("accept() error");  

	printf("---------------------------\n");
	printf("Address Conversion Server\n");
	printf("---------------------------\n");

	while(1){
		read_len=read(clnt_sock,&dotted,sizeof(dotted));
		if(read_len==-1)
			error_handling("read() error!");
		
		if(dotted.cmd==2){
				printf("[Rx] QUIT message received\n");
				break;
			}
	
		printf("[Rx] Received Dotted-Decimal Address: %s\n",dotted.addr);

		dotted.cmd = 1;
		if(!inet_aton(dotted.addr,&dotted.iaddr)){
			dotted.result = 0;
			write(clnt_sock,&dotted,sizeof(dotted));
			printf("[Tx] Address conveson fail:(%s)\n",dotted.addr);
		}
		else{
			dotted.result = 1;
			write(clnt_sock,&dotted,sizeof(dotted));
			printf("inet_aton(%s) -> %#x\n",dotted.addr,dotted.iaddr.s_addr);
			printf("[Tx] cmd :%d, iaddr: %#x, result: %d\n", dotted.cmd,dotted.iaddr.s_addr,dotted.result);
		}
		printf("\n");
	}

	close(clnt_sock);
	close(serv_sock);
	printf("Server socket close and exit.\n");
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

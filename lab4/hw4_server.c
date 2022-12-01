//2021115737 hajaehyeon
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

#define	BUF_SIZE	100
#define	SEQ_START	1000
typedef	struct	{
    int	seq; 										//	SEQ	number
    int	ack; 										//	ACK	number
    int	buf_len;								//	File	read/write	bytes
    char buf[BUF_SIZE]; //	파일이름 또는 파일 내용 전송
}Packet;

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serv_sock;
	int clnt_sock;
	int read_len=0;
    int size=0;
    int fd;
    char filename[BUF_SIZE];

	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
    Packet packet;
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
	printf("File Transmission Server\n");
	printf("---------------------------\n");

	read_len=read(clnt_sock,&packet,sizeof(packet));
	if(read_len==-1)
		error_handling("read() error!");

	strcpy(filename,packet.buf);

    fd = open(packet.buf,O_RDONLY);
	if(fd == -1){
        strcpy(packet.buf,"File Not Found");
        write(clnt_sock,&packet,sizeof(packet));

		fputs(filename, stderr);
		fputs(" ",stderr);
		error_handling(packet.buf);
    }

	printf("[Server] sending %s\n\n",packet.buf);    
    packet.seq = SEQ_START;
    while(packet.buf_len = read(fd,packet.buf,BUF_SIZE)){
		if(packet.buf_len==-1)
			error_handling("read() error");

		size+=packet.buf_len;

        printf("[Server] Tx: SEQ: %d, %d byte data\n",packet.seq, packet.buf_len);
		write(clnt_sock,&packet,sizeof(packet));
		
        read_len=read(clnt_sock,&packet,sizeof(packet));
	    if(read_len==-1)
		    error_handling("read() error!");
        packet.seq = packet.ack;

        if(packet.buf_len == 100)
            printf("[Server] Rx: ACK: %d\n",packet.ack);
		else
			break;
        printf("\n");
    }
	printf("%s sent (%d Bytes)\n",filename,size);
	close(clnt_sock);
	close(serv_sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

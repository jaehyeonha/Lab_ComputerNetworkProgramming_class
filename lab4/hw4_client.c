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

int main(int argc, char* argv[])
{
	int sock;
    int fd;
    int read_len=0;
    int size=0;
    char filename[BUF_SIZE];
	
    struct sockaddr_in serv_addr;
    Packet packet;

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

    printf("Input file name: ");
    scanf("%s",packet.buf);
    strcpy(filename,packet.buf);
    printf("[Client] request %s\n\n", filename);
    
    write(sock,&packet,sizeof(packet));

	fd = open(filename,O_CREAT|O_WRONLY|O_TRUNC,0644);

    while(read_len = read(sock,&packet,sizeof(packet))){
        if(read_len ==-1)
		    error_handling("read() error!");
        
        if(!strcmp(packet.buf,"File Not Found"))
            error_handling(packet.buf);

        size += packet.buf_len;
        write(fd,packet.buf,packet.buf_len);
        printf("[Client] Rx: SEQ: %d, len: %d bytes\n",packet.seq, packet.buf_len);

        if(packet.buf_len==100){
            packet.ack = packet.seq + packet.buf_len + 1;
            printf("[Client] Tx: ACK: %d\n", packet.ack);
            write(sock,&packet,sizeof(packet));
        }
        else
            break;

        printf("\n");   

    }
    printf("%s sent (%d Bytes)\n",filename,size);
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

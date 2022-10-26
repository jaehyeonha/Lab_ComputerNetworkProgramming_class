//2021115737 hajaehyeon
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

#define BUF_SIZE 1024
//	cmd	type	
#define FILE_REQ 0
#define FILE_RES 1
#define FILE_END 2
#define FILE_END_ACK 3
#define FILE_NOT_FOUND 4
typedef struct {
	int cmd;	
	int buf_len; //	실제 전송되는 파일의 크기 저장
	char buf[BUF_SIZE];
}PACKET;
void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sock;
    struct sockaddr_in serv_adr, from_adr;
	socklen_t adr_sz;

    PACKET packet;
    int size = 0;
    int count = 0;

    int fd;
    char filename[BUF_SIZE];

	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_DGRAM, 0);   
	if(sock==-1)
		error_handling("socket() error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));
	
    memset(&packet, 0, sizeof(packet));
    packet.cmd = FILE_REQ;
	printf("Input file name: ");
    scanf("%s",packet.buf);
    printf("[Tx] cmd: %d, file_name: %s\n",packet.cmd, packet.buf);
    strcpy(filename,packet.buf);

    sendto(sock, &packet, sizeof(packet), 0, 
								(struct sockaddr*)&serv_adr, sizeof(serv_adr));
    
    adr_sz=sizeof(from_adr);
    while(recvfrom(sock, &packet, sizeof(packet), 0, 
								(struct sockaddr*)&from_adr, &adr_sz)){
        if(packet.cmd==FILE_NOT_FOUND){
            printf("[Rx] cmd: %d, %s: %s\n",packet.cmd,filename,packet.buf);
            break;
        }

        size+=packet.buf_len;
        count+=1;
        if(packet.cmd==FILE_RES){
            printf("%s",packet.buf);
            memset(&packet, 0, sizeof(packet));
        }
        else if(packet.cmd==FILE_END){
            packet.cmd=FILE_END_ACK;
            printf("%s\n",packet.buf);
            strcpy(packet.buf,"FILE_END_ACK");
            printf("[Tx] cmd: %d, %s\n",packet.cmd, packet.buf);
            sendto(sock, &packet, sizeof(packet), 0, 
								(struct sockaddr*)&serv_adr, sizeof(serv_adr));
            break;
        } 
        memset(&packet, 0, sizeof(packet));
    }
    printf("----------------------------------------\n");
    printf("Total Tx count: %d, bytes: %d\n", count, size);
    printf("UDP Server Socket Close!\n");
    printf("----------------------------------------\n");
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
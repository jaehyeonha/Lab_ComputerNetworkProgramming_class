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
#define FILE_NAME_REQ 0
#define FILE_NAME_RES 1
#define FILE_REQ 2
#define FILE_SEND 3
#define FILE_END 4
#define FILE_END_ACK 5

#define SUCCESS	0
#define FAIL -1

typedef struct {
	int cmd;	
	int buf_len; 
	char buf[BUF_SIZE];
	int result;	
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
    packet.cmd = FILE_NAME_REQ;
	printf("Input file name: ");
    scanf("%s",packet.buf);
    printf("[Tx] FILE_NAME_REQ(cmd: %d), file_name: %s\n",packet.cmd, packet.buf);
    strcpy(filename,packet.buf);

    sendto(sock, &packet, sizeof(packet), 0, 
								(struct sockaddr*)&serv_adr, sizeof(serv_adr));
    
    adr_sz=sizeof(from_adr);
    recvfrom(sock, &packet, sizeof(packet), 0, 
								(struct sockaddr*)&from_adr, &adr_sz);
    if(packet.result==FAIL&&packet.cmd==FILE_NAME_RES){
        printf("[Rx] FILE_NAME_RES(cmd: %d), %s: %s\n",packet.cmd,filename,packet.buf);
    }
    else if(packet.result==SUCCESS&&packet.cmd==FILE_NAME_RES){
        packet.cmd = FILE_REQ;
        sendto(sock, &packet, sizeof(packet), 0, 
                                    (struct sockaddr*)&serv_adr, sizeof(serv_adr));

        adr_sz=sizeof(from_adr);
        while(recvfrom(sock, &packet, sizeof(packet), 0, 
								(struct sockaddr*)&from_adr, &adr_sz)){
            size+=packet.buf_len;
            count+=1;
            if(packet.cmd == FILE_SEND){
                packet.cmd = FILE_REQ;
                printf("%s",packet.buf);
                sendto(sock, &packet, sizeof(packet), 0, 
                                    (struct sockaddr*)&serv_adr, sizeof(serv_adr));
            }
            else if(packet.cmd==FILE_END){
                packet.cmd=FILE_END_ACK;
                for(int i=0; i<packet.buf_len; i++){
                    printf("%c",packet.buf[i]);
                }
                sendto(sock, &packet, sizeof(packet), 0, 
                                    (struct sockaddr*)&serv_adr, sizeof(serv_adr));
                break;
            } 
            memset(&packet, 0, sizeof(packet));
        }
    }
    printf("\n");
    printf("----------------------------------------\n");
    printf("Total Tx count: %d, bytes: %d\n", count, size);
    printf("UDP Server Socket Close!\n");
    printf("----------------------------------------\n");
    fclose(fd);
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
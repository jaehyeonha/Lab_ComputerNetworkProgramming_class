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
	int serv_sock;
    struct sockaddr_in serv_adr, clnt_adr;
	socklen_t clnt_adr_sz;
	
    PACKET packet;
    int size = 0;
    int count = 0;

    int fd;
    char filename[BUF_SIZE];

	if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	serv_sock=socket(PF_INET, SOCK_DGRAM, 0);
	if(serv_sock==-1)
		error_handling("UDP socket creation error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");

    memset(&packet, 0, sizeof(packet));
    clnt_adr_sz=sizeof(clnt_adr);
	recvfrom(serv_sock, &packet, sizeof(packet), 0, 
								(struct sockaddr*)&clnt_adr, &clnt_adr_sz);
    if(packet.cmd == FILE_NAME_REQ){
        printf("[Rx] FILE_NAME_REQ(cmd: %d), file_name: %s\n",packet.cmd, packet.buf);
        strcpy(filename,packet.buf);
    }
    
    packet.cmd = FILE_NAME_RES;
    fd = open(packet.buf,O_RDONLY);
	if(fd == -1){
        strcpy(packet.buf,"File Not Found");
        packet.result = FAIL;
        printf("%s %s\n",filename,packet.buf);
        printf("[Tx] FILE_NAME_RES(cmd: %d), FILE_NAME_RES, result:%d\n",packet.cmd, packet.result);
        sendto(serv_sock, &packet, sizeof(packet), 0, 
								(struct sockaddr*)&clnt_adr, clnt_adr_sz);
    }
    else{
        packet.result = SUCCESS;
        printf("[Tx] FILE_NAME_RES(cmd: %d), FILE_NAME_RES, result:%d\n",packet.cmd, packet.result);
        sendto(serv_sock, &packet, sizeof(packet), 0, 
								(struct sockaddr*)&clnt_adr, clnt_adr_sz);
        
        clnt_adr_sz=sizeof(clnt_adr);
        while(recvfrom(serv_sock, &packet, sizeof(packet), 0, 
								(struct sockaddr*)&clnt_adr, &clnt_adr_sz)){
            if(packet.cmd==FILE_REQ){
                printf("[Rx] FILE_NAME_REQ(cmd: %d)\n",packet.cmd);
                packet.buf_len = read(fd,packet.buf,BUF_SIZE);
		        if(packet.buf_len==-1)
			        error_handling("read() error");

                size+=packet.buf_len;
                count+=1;
            }
            sleep(1);

            if(packet.buf_len == BUF_SIZE){
                packet.cmd = FILE_SEND;
                printf("[Tx] FILE_SEND(cmd: %d) len: %d, totla_tx_cnt: %d, total_tx_bytes: %d\n",packet.cmd, packet.buf_len, count, size);
                sendto(serv_sock, &packet, sizeof(packet), 0, 
								(struct sockaddr*)&clnt_adr, clnt_adr_sz);
            }
            else if(packet.buf_len != BUF_SIZE){
                packet.cmd = FILE_END;
                printf("[Tx] FILE_END(cmd: %d) len: %d, totla_tx_cnt: %d, total_tx_bytes: %d\n",packet.cmd, packet.buf_len, count, size);
                sendto(serv_sock, &packet, sizeof(packet), 0, 
								(struct sockaddr*)&clnt_adr, clnt_adr_sz);
                clnt_adr_sz=sizeof(clnt_adr);
                recvfrom(serv_sock, &packet, sizeof(packet), 0, 
								(struct sockaddr*)&clnt_adr, &clnt_adr_sz);
                if(packet.cmd == FILE_END_ACK){
                    printf("[Rx] FILE_END_ACK(cmd: %d)\n",packet.cmd);
                    break;
                }
            }
            memset(&packet, 0, sizeof(packet));
        }
    }
    printf("\n");
    printf("----------------------------------------\n");
    printf("Total Tx count: %d, bytes: %d\n", count, size);
    printf("UDP Server Socket Close!\n");
    printf("----------------------------------------\n");
    close(fd);
	close(serv_sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

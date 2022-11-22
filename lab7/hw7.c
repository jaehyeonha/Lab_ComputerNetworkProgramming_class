//2021115737 hajaehyeon
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define TTL 64
#define BUF_SIZE 120
void error_handling(char *message);

int main(int argc, char *argv[])
{
    int send_sock;
    int recv_sock;
    int str_len;
    struct sockaddr_in mul_adr;
    struct sockaddr_in adr;
    struct ip_mreq join_adr;
    int time_live = TTL;
    char buf[BUF_SIZE];

    if(argc != 4) {
        printf("Usage : %s <GroupIP> <PORT> <Name>\n", argv[0]);
        exit(1);
    }

    send_sock = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&mul_adr, 0, sizeof(mul_adr));
    mul_adr.sin_family = AF_INET;
    mul_adr.sin_addr.s_addr = inet_addr(argv[1]);
    mul_adr.sin_port=htons(atoi(argv[2]));

    setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&time_live, sizeof(time_live));

    recv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&adr, 0, sizeof(adr));
    adr.sin_family = AF_INET;
    adr.sin_addr.s_addr = htonl(INADDR_ANY);
    adr.sin_port = htons(atoi(argv[2]));

    int reuse=1;

    setsockopt(recv_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));

    if(bind(recv_sock, (struct sockaddr*)&adr, sizeof(adr)) == -1)
        error_handling("bind() error");

    join_adr.imr_multiaddr.s_addr = inet_addr(argv[1]);    
    join_adr.imr_interface.s_addr = htonl(INADDR_ANY);     

    setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&join_adr, sizeof(join_adr));
    
    pid_t pid = fork();

    if(pid==0){
        while(1){
            memset(buf, 0, sizeof(buf));
            str_len = recvfrom(recv_sock, buf, BUF_SIZE, 0, NULL, 0);
            printf("Receiced Message: %s", buf);
        }
    }
    else{   
        char message[102];  
        while (fgets(message, 102, stdin) != NULL){
            if(message[100]!='\n' && message[100]!='\0'){
                printf("max 100 retry\n");
                while(1){
                    char c = getchar();
                    if(c=='\n'){
                        memset(message, 0, sizeof(message));
                        break;
                    }
                }
                continue;
            }
            if(!strcmp(message,"q\n") || !strcmp( message, "Q\n")){
                setsockopt(recv_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&join_adr, sizeof(join_adr));
                printf("Leave multicast group\n");
                break;
            }
            sprintf(buf, "[%s] %s", argv[3], message);
            sendto(send_sock, buf, strlen(buf), 0, (struct sockaddr *)&mul_adr, sizeof(mul_adr));
            memset(buf, 0, sizeof(buf));
            memset(message, 0, sizeof(message));
        }
    }
    
    close(send_sock);
    close(recv_sock);
    return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

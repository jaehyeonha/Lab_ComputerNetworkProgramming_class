//2021115737 hajaehyeon
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>

#define BUF_SIZE 2048
void error_handling(char *buf);

int main(int argc, char *argv[])
{
    int fd1, fd2;
	struct sockaddr_in serv_adr;
    struct timeval timeout;
    fd_set reads, cpy_reads;

    int fd_max, str_len, fd_num;
    char buf[BUF_SIZE];
    int choose;

	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
    printf("-----------------------------------------\n");
    printf("Choose function\n");
    printf("1. Sender,   2: Receiver\n");
    printf("-----------------------------------------\n");
    printf(" => ");
    scanf("%d", &choose);

    FD_ZERO(&reads);

    if(choose == 1){
        printf("File Sender Start!\n");
        fd1 = open("rfc1180.txt", O_RDONLY);
		if(fd1 == -1)
			error_handling("open() error");
		FD_SET(fd1, &reads);
    }
    else if(choose == 2){
        printf("File Receiver Start!\n");
    }

    fd2 = socket(PF_INET, SOCK_STREAM, 0);   
	if(fd2==-1)
		error_handling("socket() error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));
	
	if(connect(fd2, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("connect() error!");
	else
		puts("Connected...........");

    if(choose == 1){
        write(fd2, "Sender", sizeof("Sender"));
        printf("fd1 : %d, fd2: %d\n", fd1, fd2);
    }
    else if(choose == 2){
        write(fd2, "Receiver", sizeof("Receiver"));
        printf("fd2: %d\n", fd2);
    }

    FD_SET(fd2, &reads);
    fd_max = fd2;
    printf("max_fd: %d\n",fd_max);

	while(1) 
	{
        cpy_reads = reads;
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        memset(buf, 0, sizeof(buf));

        if((fd_num = select(fd_max+1, &cpy_reads, 0, 0, &timeout))== -1)
            break;
        if(fd_num == 0)
            continue;

        
        if(choose == 1){
            if(FD_ISSET(fd1, &cpy_reads)){
                str_len = read(fd1, buf, BUF_SIZE);

                if(str_len == 0){
                    FD_CLR(fd1, &reads);
                    close(fd1);
                }
                sleep(1);
                write(fd2, buf, str_len);
            }
            if(FD_ISSET(fd2, &cpy_reads)){
                read(fd2, buf, BUF_SIZE);
                printf("%s",buf);
            }
        }
        else if(choose == 2){
             if(FD_ISSET(fd2, &cpy_reads)){
                str_len = read(fd2, buf, BUF_SIZE);
                printf("%s",buf);
                write(fd2, buf, str_len);
            }
        }
	}

	FD_CLR(fd2, &reads);
	close(fd2);
	return 0;
}

void error_handling(char *buf)
{
	fputs(buf, stderr);
	fputc('\n', stderr);
	exit(1);
}
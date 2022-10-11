#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#define BUF_SIZE 100
void error_handling(char* message);

int main(int argc, char *argv[])
{
	int lfd,rfd;
	int idx=0;
	int size=0;
	int rsize,lsize;
	char buf[BUF_SIZE];

	if(argc!=3){
		printf("[Error] mymove Usage: %s src_file dest_file\n",argv[0]);
		exit(1);
	}
	rfd = open(argv[1],O_RDONLY);
	lfd = open(argv[2],O_CREAT|O_WRONLY|O_TRUNC,0644);
	if(rfd == -1)
		error_handling("open() error!");
	if(lfd == -1)
		error_handling("open() error!");

	while(rsize=read(rfd,buf,sizeof(buf))){
		if(rsize==-1){
			error_handling("read() error");
			break;
		}
		size+=rsize;
		lsize=write(lfd,buf,rsize);
		if(lsize == -1){
			error_handling("write() error");
		}
	}
	
	close(rfd);
	close(lfd);
	
	printf("move from %s to %s (bytes: %d) finished.\n",argv[1],argv[2],size);

	remove(argv[1]);
	return 0;
}

void error_handling(char *message)
{
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}

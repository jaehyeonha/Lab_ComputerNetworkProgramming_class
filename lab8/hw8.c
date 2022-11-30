//2021115737 hajaehyeon
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    FILE * readfp1;
	FILE * readfp2;
    char buf1[BUF_SIZE], buf2[BUF_SIZE];

    int fd = open ("data1.txt", O_RDONLY);	

	readfp1=fdopen(fd, "r");
	readfp2=fdopen(dup(fd), "r");
	
	while(1){
        if(!fgets(buf1, BUF_SIZE, readfp1))
            break;
        fputs(buf1, stdout);
        fflush(readfp1);
        if(!fgets(buf2, BUF_SIZE, readfp2))
            break;
        fputs(buf2, stdout);
        fflush(readfp2);
    }

	fclose(readfp1);
    fclose(readfp2);
	return 0;
}



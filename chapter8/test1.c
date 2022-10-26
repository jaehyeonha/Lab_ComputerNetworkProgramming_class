#include <stdio.h>
#include <string.h>

#define BUF_SIZE 1024

int main()
{
	char msg[BUF_SIZE];
	int len = 0;
	int i;

	printf("Input String: ");
	//scanf("%s", msg);
	fgets(msg, BUF_SIZE, stdin);

	len = strlen(msg);
	msg[len] = 0;
	printf("strlen(msg): %d\n", len);

	if(strcmp(msg, "quit") == 0)
	{
		printf("%s!\n", msg);
		return -1;
	}

	for(i=0; i<len; i++)
	{
		printf("%c ", msg[i]);
	}
	printf("\n");

	return 0;
}



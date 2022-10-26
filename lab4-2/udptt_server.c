//2021115737 hajaehyeon
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <time.h>

#define INIT_VALUE 0
#define S_VALUE 1
#define C_VALUE 2

#define BOARD_SIZE 3
typedef struct {
    int board[BOARD_SIZE][BOARD_SIZE];
}GAMEBOARD;

void error_handling(char *message);
int availble_space(GAMEBOARD *gboard);
void draw_board(GAMEBOARD *gboard);

int main(int argc, char *argv[])
{
	int serv_sock;
    struct sockaddr_in serv_adr, clnt_adr;
	socklen_t clnt_adr_sz;

    GAMEBOARD gameboard;
    int row, col;
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

    printf("Tic-Tac-Toe Server\n");
    memset(&gameboard.board, INIT_VALUE, sizeof(gameboard.board)); 
    draw_board(&gameboard);

    while(1){
        clnt_adr_sz=sizeof(clnt_adr);
        recvfrom(serv_sock, &gameboard, sizeof(gameboard), 0, 
								(struct sockaddr*)&clnt_adr, &clnt_adr_sz);
        draw_board(&gameboard);
        if(!availble_space(&gameboard)){
            printf("No available space. Exit this program.\n");
            break;
        }

        while(1){
            srand(time(NULL));
            row = rand() % BOARD_SIZE;
            col = rand() % BOARD_SIZE;
            if (!gameboard.board[row][col]){
                gameboard.board[row][col] = S_VALUE;
                printf("Server Choose: [%d, %d]\n",row,col);
                break;
            }
        }
        sendto(serv_sock, &gameboard, sizeof(gameboard), 0, 
								(struct sockaddr*)&clnt_adr, clnt_adr_sz);
        draw_board(&gameboard);
    }

    printf("Tic Tac Toe Server Close\n");
    close(serv_sock);
}

int availble_space(GAMEBOARD *gboard){
    int res = 0;
    for(int i = 0; i< BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            if (gboard->board[i][j] == INIT_VALUE)
                return 1;
        }
    }
    return 0;
}

void draw_board(GAMEBOARD *gboard){
    char value = ' ';
    int i, j;
    printf("+-----------+\n");
    for(i = 0; i < BOARD_SIZE; i++){
        for (j = 0; j < BOARD_SIZE; j++){
            if (gboard->board[i][j] == INIT_VALUE)
                value = ' ';
            else if (gboard->board[i][j] == S_VALUE)
                value = 'O';
            else if (gboard->board[i][j] == C_VALUE)
                value = 'X';
            else value = ' ';
            printf("| %c ", value);
        }
        printf("|");
         printf("\n+-----------+\n");
    }
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
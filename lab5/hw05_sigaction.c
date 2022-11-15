#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
int elapsedtime = 0;

void pTimeout(int sig){
   elapsedtime += 2;
   if(sig==SIGALRM)
      printf("<Parent> time out: 2, elapsed time: %d seconds\n", elapsedtime);
   alarm(2);
}

void cTimeout(int sig){
   elapsedtime += 5;
   if(sig==SIGALRM)
      printf("[Child] time out: 5, elapsed time: %d seconds\n", elapsedtime);
   alarm(5);
}

void read_childproc(int sig){
    int status;
    pid_t id = waitpid(-1,&status,WNOHANG);
    if(WIFEXITED(status))
    {
        printf("Child id: %d, sent: %d\n",id, WEXITSTATUS(status));
    }
}

int main(int argc, char* argv[])
{

    struct sigaction finishact;
    sigemptyset(&finishact.sa_mask);
    finishact.sa_flags=0;
    finishact.sa_handler = read_childproc;
    sigaction(SIGCHLD,&finishact,0);
    
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    
    pid_t pid;
    pid = fork();

    if(pid==0){
        act.sa_handler = cTimeout;
        sigaction(SIGALRM,&act,0);
        alarm(5);
        for(int i = 0; i<20; i++)
            sleep(1);
        return 5;
    }
    else{
        act.sa_handler = pTimeout;
        sigaction(SIGALRM,&act,0);
        alarm(2);
        for(;;)
            sleep(1);
    }
    return 0;
}
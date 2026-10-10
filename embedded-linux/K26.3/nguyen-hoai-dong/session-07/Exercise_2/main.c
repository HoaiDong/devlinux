#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

sigset_t block_set;

void handle_sigusr1(int sig)
{
    printf("[GATEWAY] Worker reported READY signal received.\n");
}

int main(void)
{
    if(signal(SIGUSR1, handle_sigusr1) != 0)
    {
        printf("[ERROR] Initial SIGUSR1 Fail\n");
    }
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGUSR1);
    pid_t pid = fork();
    if(pid == 0)
    {
        fflush(stdout);
        sleep(2);
        kill(getppid(), SIGUSR1);
        printf("[WORKER] Sent READY signal to gateway\n");
        exit(7);
    }
    else
    {   
        printf("[GATEWAY] Worker PID = %d\n", pid);
        sigprocmask(SIG_BLOCK, &block_set, NULL);
        sleep(5);
        sigprocmask(SIG_UNBLOCK, &block_set, NULL);
        int status;
        if(wait(&status) == -1)
        {
            printf("[ERROR] Wait children unsuccessful\n");
        }
        if(WIFEXITED(status))
        {
            if(WEXITSTATUS(status) == 7)
            {
                printf("[GATEWAY] Worker exited with code %d\n", WEXITSTATUS(status));
            }
        }
    }
    return 0;
}
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

sigset_t block_set, old_set;

int main(void)
{
    if(sigemptyset(&block_set) != 0)
    {
        printf("[ERROR] sigemptyset is error or invalid sig number\n");
        return 1;
    }
    if(sigaddset(&block_set, SIGINT) != 0)
    {
        printf("[ERROR] sigaddset is error\n");
        return 1;
    }
    
    for(int i = 0; i < 5; i++)
    {
        if(sigprocmask(SIG_BLOCK, &block_set, &old_set) != 0)
        {
            printf("[ERROR] sigprocmask is error\n");
            exit(1);
        }
        printf("[SAFE] Writing transaction #%d ...\n", i + 1);
        sleep(3);
        printf("[SAFE] Transaction #%d committed.\n", i + 1);
        if(sigprocmask(SIG_SETMASK, &old_set, NULL) != 0)
        {
            printf("[ERROR] sigprocmask is error\n");
            exit(1);
        }
        printf("[IDLE] Waiting for next transaction...\n");
        sleep(3);
    }
    return 0;
}
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int loop = 1;

void handler(int sig)
{
    if (sig == SIGTERM)
    {
        loop = 0;
    }
}

int main(void)
{
    setbuf(stdout, NULL);
    signal (SIGTERM, handler);

    while (loop)
    {
        printf("RUNNING...\n");
        sleep(1);
    }
    printf("Service shutting down...\n");
    return 0;
}
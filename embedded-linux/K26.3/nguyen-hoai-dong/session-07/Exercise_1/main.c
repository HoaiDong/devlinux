#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

volatile sig_atomic_t reading_count = 0;

void handle_sigint(int sig)
{
    printf("[WARN] Received SIGINT, ignoring...\n");
}

void handle_sigterm(int sig)
{
    printf("[INFO] Received SIGTERM, shutting down gracefully...\n");
    exit(0);
}

void handle_sigusr1(int sig)
{
    printf("[REPORT] Total readings so far: %d\n", reading_count);
}

float random_double(float min, float max)
{
    return min + (max - min) * ((float)rand() / RAND_MAX);
}

int main(void)
{
    if(signal(SIGINT, handle_sigint) != 0)
    {
        printf("[ERROR] FAIL INITIAL SIGINT\n");
        return 1;
    }
        
    if(signal(SIGTERM, handle_sigterm) != 0)
    {
        printf("[ERROR] FAIL INITIAL SIGTERM\n");
        return 1;
    }
        
    if(signal(SIGUSR1, handle_sigusr1) != 0)
    {
        printf("[ERROR] FAIL INITIAL SIGUSR1\n");
        return 1;
    }
          

    while(1)
    {
        printf("[INFO] [PID: %d] Sensor reading #%d: temperature=%0.2f\n", getpid(), reading_count, random_double(0,100));
        reading_count++;
        sleep(1);
    }
    return 0;
}
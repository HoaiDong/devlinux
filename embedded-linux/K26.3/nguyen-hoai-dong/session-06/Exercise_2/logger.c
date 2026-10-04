#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define LOG_ERR     "<3>"
#define LOG_WARNING "<4>"
#define LOG_INFO    "<6>"

void *thread_err(void *arg)
{
    int cycle = 1;

    while (1)
    {
        fprintf(stderr, LOG_INFO    "Service running normally, cycle %d\n", cycle);
        fprintf(stderr, LOG_WARNING "Memory usage high: %d%%\n", 80 + rand() % 15);
        fprintf(stderr, LOG_ERR     "Failed to connect to database, retry %d\n", cycle);
        sleep(2);
        cycle++;
    }
}

void *thread_abort(void *arg)
{
    sleep(30);
    abort();
}

int main(void)
{
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    pthread_t thread_e, thread_a;
    if (pthread_create(&thread_e, NULL, thread_err, NULL) != 0)
    {
        fprintf(stderr, LOG_ERR    "Create p_thread thread_err unsuccessfull\n");
        exit(1);
    }
    if (pthread_create(&thread_a, NULL, thread_abort, NULL) != 0)
    {
        fprintf(stderr, LOG_ERR    "Create p_thread thread_abort unsuccessfull\n");
        exit(1);
    }

    pthread_join(thread_e, NULL);
    pthread_join(thread_a, NULL);
    return 0;
}
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "data_type.h"
#include "log_process.h"

gateway_t gateway_main;
pthread_mutex_t mutex_stop_flag = PTHREAD_MUTEX_INITIALIZER;

static void debug_log(char *input)
{
    (void)input;
    #ifdef DEBUG
    printf("%s\n", input);
    #endif
}

int check_stop_flag(void)
{
    int status_flag = 0;
    pthread_mutex_lock(&mutex_stop_flag);
    status_flag = gateway_main.stop;
    pthread_mutex_unlock(&mutex_stop_flag);
    return status_flag;
}

void *pthread_connection_manager(void *arg)
{
    (void)arg;

    debug_log("[Connection][Thread] Start");
    send_log("CONN", "Start");
    while (1)
    {    
        debug_log("[Connection][Thread] running...");
        send_log("CONN", "running...");

        if(check_stop_flag() == 1)
            break;

        sleep(DELAY_TIME);
    }
    debug_log("[Connection][Thread] end");
    send_log("CONN", "End");
    return NULL;
}

void *pthread_data_manager(void *arg)
{
    (void)arg;
    
    debug_log("[Data][Thread] Start");
    send_log("DATA", "Start");
    while (1)
    {
        debug_log("[Data][Thread] Running...");
        send_log("DATA", "running...");

        if(check_stop_flag() == 1)
            break;

        sleep(DELAY_TIME);
    }
    debug_log("[Data][Thread] End");
    send_log("DATA", "End");
    return NULL;
}

void *pthread_storage_manager(void *arg)
{
    (void)arg; //solve warning: unused parameter ‘arg’
    
    debug_log("[Storage][Thread] Start");
    send_log("STOR", "Start");
    while (1)
    {
        debug_log("[Storage][Thread] Running...");
        send_log("STOR", "running...");

        if(check_stop_flag() == 1)
            break;

        sleep(DELAY_TIME);
    }
    debug_log("[Storage][Thread] End");
    send_log("STOR", "End");
    return NULL; //solve: warning: control reaches end of non-void function
}

void check_input_argument(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("[ERROR] Missing or invalid argc\n");
        exit(1);
    }
    debug_log(argv[1]);
    debug_log(argv[2]);

    for (int i = 0; argv[1][i] != 0; i++)
    {
        if (argv[1][i] < '0' || argv[1][i] > '9')
        {
            printf("[ERROR] Invalid argv 1\n");
            exit(1);
        }
    }

    for (int i = 0; argv[2][i] != 0; i++)
    {
        if (argv[2][i] < '0' || argv[2][i] > '9')
        {
            printf("[ERROR] Invalid argv 2\n");
            exit(1);
        }
    }

    int port = atoi(argv[1]);
    int http = atoi(argv[2]);
    
    if (port < 1 || port > 65532 ||
        http < 1 || http > 65532)
    {
        printf("[ERROR] Invalid argv\n");
        exit(1);
    }

    char temp[70];
    snprintf(temp, sizeof(temp), "Gateway started (TCP base port %d, HTTP port %d)", port, http);
    send_log("MAIN", temp);
}

static void sigint_hanlder(int sig)
{
    (void)sig;
    debug_log("[MAIN] SIGINT occur...");
    pthread_mutex_lock(&mutex_stop_flag);
    gateway_main.stop = 1;
    pthread_mutex_unlock(&mutex_stop_flag);
}

static void sigterm_hanlder(int sig)
{
    (void)sig;
    debug_log("[MAIN] SIGTERM occur...");
    pthread_mutex_lock(&mutex_stop_flag);
    gateway_main.stop = 1;
    pthread_mutex_unlock(&mutex_stop_flag);
}

static void init_signal(void)
{
    if (signal(SIGINT, sigint_hanlder) != 0)
    {
        printf("[ERROR] Init SIGINT error\n");
        exit(1);
    }
    if (signal(SIGTERM, sigterm_hanlder) != 0)
    {
        printf("[ERROR] Init SIGTERM error\n");
        exit(1);
    }
}

void create_thread(pthread_t *thread_manager)
{
    pthread_create(&thread_manager[THREAD_CONNECTION], NULL, pthread_connection_manager, NULL);
    pthread_create(&thread_manager[THREAD_DATA], NULL, pthread_data_manager, NULL);
    pthread_create(&thread_manager[THREAD_STORAGE], NULL, pthread_storage_manager, NULL);
}

void join_thread(pthread_t *thread_manager)
{
    for(int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(thread_manager[i], NULL);
    }
}


int main(int argc, char *argv[])
{
    
    debug_log("[MAIN] Start");
    fork_log_process();
    check_input_argument(argc, argv);

    gateway_main.stop = 0;
    pthread_t thread_manager[THREAD_COUNT];
    create_thread(thread_manager);

    pthread_mutex_init(&mutex_stop_flag, NULL);
    init_signal();

    while(1)
    {
        sleep(DELAY_TIME);
        debug_log("[MAIN] Running...");
        if(check_stop_flag() == 1)
            break;          
    }

    join_thread(thread_manager);
    pthread_mutex_destroy(&mutex_stop_flag);

    send_log("MAIN", "Gateway shutting down");
    wait_log_process();
    debug_log("Gateway shutting down...");
    debug_log("[MAIN] End");
    return 0;
}
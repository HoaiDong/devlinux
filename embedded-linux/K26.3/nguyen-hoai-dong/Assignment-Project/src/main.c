#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "data_type.h"

gateway_t gateway_main;
pthread_mutex_t mutex_stop_flag = PTHREAD_MUTEX_INITIALIZER;

void debug_log(char *input)
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
    debug_log("Checking flag");
    pthread_mutex_unlock(&mutex_stop_flag);
    debug_log("Checking flag compleeted");
    return status_flag;
}

void *pthread_connection_manager(void *arg)
{
    (void)arg;

    debug_log("pthread_connection_manager");
    while (1)
    {    

        debug_log("pthread_connection_manager running...");
        sleep(1);

        if(check_stop_flag() == 1)
            break;
    }
    debug_log("pthread_connection_manager stopped.");
    return NULL;
}

void *pthread_data_manager(void *arg)
{
    (void)arg;
    
    debug_log("pthread_data_manager");
    while (1)
    {
        debug_log("pthread_data_manager running...");
        sleep(1);

        if(check_stop_flag() == 1)
            break;
    }
    debug_log("pthread_data_manager stopped.");
    return NULL;
}

void *pthread_storage_manager(void *arg)
{
    (void)arg; //solve warning: unused parameter ‘arg’
    

    debug_log("pthread_storage_manager");
    while (1)
    {
        debug_log("pthread_storage_manager running...");
        sleep(1);

        if(check_stop_flag() == 1)
            break;
    }
    debug_log("pthread_storage_manager stopped.");
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
}

void sigint_hanlder(int sig)
{
    (void)sig;
    debug_log("SIGINT occur...");
    pthread_mutex_lock(&mutex_stop_flag);
    gateway_main.stop = 1;
    pthread_mutex_unlock(&mutex_stop_flag);
}

void sigterm_hanlder(int sig)
{
    (void)sig;
    debug_log("SIGTERM occur...");
    pthread_mutex_lock(&mutex_stop_flag);
    gateway_main.stop = 1;
    pthread_mutex_unlock(&mutex_stop_flag);
}

void init_signal(void)
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
int main(int argc, char *argv[])
{
    debug_log("Gateway started...");
    check_input_argument(argc, argv);
    
    gateway_main.stop = 0;
    pthread_t pthread_connection, pthread_data, pthread_storage;
    pthread_mutex_init(&mutex_stop_flag, NULL);
    init_signal();
    
    pthread_create(&pthread_connection, NULL, pthread_connection_manager, NULL);
    pthread_create(&pthread_data, NULL, pthread_data_manager, NULL);
    pthread_create(&pthread_storage, NULL, pthread_storage_manager, NULL);

    while(1)
    {
        sleep(2);
        if(check_stop_flag() == 1)
            break;          
    }
    
    pthread_join(pthread_connection, NULL);
    pthread_join(pthread_data, NULL);
    pthread_join(pthread_storage, NULL);

    pthread_mutex_destroy(&mutex_stop_flag);
    debug_log("Gateway shutting down...");
    return 0;
}
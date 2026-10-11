#include <stdio.h>
#include <pthread.h>

void debug_log(char *input)
{
    (void)input;
    #ifdef DEBUG
    printf("%s\n", input);
    #endif
}

void *pthread_connection_manager(void *arg)
{
    (void)arg;

    debug_log("pthread_connection_manager");
    return NULL;
}

void *pthread_data_manager(void *arg)
{
    (void)arg;
    
    debug_log("pthread_data_manager");
    return NULL;
}

void *pthread_storage_manager(void *arg)
{
    (void)arg; //solve warning: unused parameter ‘arg’
    

    debug_log("pthread_storage_manager");
    return NULL; //solve: warning: control reaches end of non-void function
}

int main(void)
{
    debug_log("Gateway started...");

    pthread_t pthread_connection, pthread_data, pthread_storage;

    pthread_create(&pthread_connection, NULL, pthread_connection_manager, NULL);
    pthread_create(&pthread_data, NULL, pthread_data_manager, NULL);
    pthread_create(&pthread_storage, NULL, pthread_storage_manager, NULL);

    pthread_join(pthread_connection, NULL);
    pthread_join(pthread_data, NULL);
    pthread_join(pthread_storage, NULL);

    debug_log("Gateway shutting down...");
    return 0;
}
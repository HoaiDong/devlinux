#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>  // socket(), bind(), listen(), accept(), send(), recv()
#include <netinet/in.h>  // struct sockaddr_in, htons(), INADDR_ANY
#include <arpa/inet.h>  // inet_pton()
#include <string.h>
#include <sys/epoll.h>

#include "data_type.h"
#include "log_process.h"

gateway_t gateway_main = {0};
pthread_mutex_t mutex_stop_flag = PTHREAD_MUTEX_INITIALIZER;
sbuffer_t sbuffer = {0};

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

void init_server_socket(void)
{
    for(int i = 0; i < MAX_LISTEN_FD; i++)
    {
        gateway_main.conn.listen_fd[i] = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(gateway_main.base_port + i);
        if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1)
        {
            debug_log("[ERROR] inet_pton error");
            close(gateway_main.conn.listen_fd[i]);
            exit(1);
        }

        if (bind(gateway_main.conn.listen_fd[i], (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) != 0)
        {
            debug_log("[ERROR] bind error");
            close(gateway_main.conn.listen_fd[i]);
            exit(1);
        }
        if (listen(gateway_main.conn.listen_fd[i], 1) != 0)
        {
            debug_log("[ERROR] listen error");
            close(gateway_main.conn.listen_fd[i]);
            exit(1);
        }
    }
}

void init_all_connection(void)
{
    for(int i = 0; i < MAX_CONN; i++)
    {
        gateway_main.conn.conns[i].fd = -1;
    }

    gateway_main.conn.n_conns = 0;
}

int get_index_client_connection(int fd)
{
    for(int i = 0; i < MAX_CONN; i++)
    {
        if(gateway_main.conn.conns[i].fd == fd)
            return i;
    }
    return -1;
}

void add_new_conenction(int fd, int *epfd)
{
    struct epoll_event sensor = {0};
    sensor.events = EPOLLIN;
    sensor.data.fd = fd;

    for(int i = 0; i < MAX_CONN; i++)
    {
        if(gateway_main.conn.conns[i].fd == -1)
        {
            gateway_main.conn.conns[i].fd = fd;
            epoll_ctl(*epfd, EPOLL_CTL_ADD, gateway_main.conn.conns[i].fd, &sensor);
            memset(gateway_main.conn.conns[i].rx, 0, RX_BUF_SIZE);
            gateway_main.conn.conns[i].rx_len = 0;
            gateway_main.conn.conns[i].identified = false;
            gateway_main.conn.conns[i].sensor_id = 0;
            break;
        }
    }

    gateway_main.conn.n_conns++;
}

void delete_connection(int fd, int *epfd)
{
    int index = get_index_client_connection(fd);
    close(fd);
    epoll_ctl(*epfd, EPOLL_CTL_DEL, fd, NULL);

    gateway_main.conn.conns[index].fd = -1;
    memset(gateway_main.conn.conns[index].rx, 0, RX_BUF_SIZE);
    gateway_main.conn.conns[index].rx_len = 0;
    gateway_main.conn.conns[index].identified = false;
    gateway_main.conn.conns[index].sensor_id = 0;

    gateway_main.conn.n_conns--;
}

void hanling_event_fd(int fd, int *epfd)
{
    for(int i = 0; i < MAX_LISTEN_FD; i++)
    {
        if (fd == gateway_main.conn.listen_fd[i])
        {
            if(gateway_main.conn.n_conns < MAX_CONN)
            {
                
                int new_fd = accept(gateway_main.conn.listen_fd[i], NULL, NULL);
                add_new_conenction(new_fd, epfd);
                return;
            }
        }
    }
    
    int index = get_index_client_connection(fd);

    int return_value = recv(fd, gateway_main.conn.conns[index].rx, RX_BUF_SIZE, 0);
    if(return_value == 0)
    {
        printf("[SENSOR] SENSOR disconnected\n");
        delete_connection(fd, epfd);
    } 
    else if (return_value == -1)
    {
        printf("[ERROR] recv error\n");
    }   
    else
    {
        uint16_t sensor_id = 0;
        double temperature = 0;
        time_t sensor_ts;

        memcpy(&sensor_id, gateway_main.conn.conns[index].rx, 2);
        memcpy(&temperature, &gateway_main.conn.conns[index].rx[2], 8);
        memcpy(&sensor_ts, &gateway_main.conn.conns[index].rx[10], 8);
        
        struct tm time_info;
        char time_format[20];
        localtime_r(&sensor_ts, &time_info);
        strftime(time_format, sizeof(time_format), "%Y-%m-%d %H:%M:%S", &time_info);

        char s[100];
        memset(s, 0, sizeof(s));
        
        snprintf(s, sizeof(s), "Sensor ID: %d - Temperature: %0.1f - Time: %s", sensor_id, temperature, time_format);
        debug_log(s);
    }
}

void *pthread_connection_manager(void *arg)
{
    (void)arg;

    debug_log("[Connection][Thread] Start");
    send_log("CONN", "Start");

    printf("Server listening port: %d\n", gateway_main.base_port);

    init_all_connection();
    init_server_socket();
   
    int epfd = epoll_create1(0);
    for(int i = 0; i < MAX_LISTEN_FD; i++)
    {
        struct epoll_event ev = {0};
        ev.events = EPOLLIN;
        ev.data.fd = gateway_main.conn.listen_fd[i];
        epoll_ctl(epfd, EPOLL_CTL_ADD, gateway_main.conn.listen_fd[i], &ev);
    }

    while(1)
    {
        struct epoll_event events[10];
        int n = epoll_wait(epfd, events, 10, 100);
        for (int i = 0; i < n; i++)
        {
            int fd = events[i].data.fd;

            hanling_event_fd(fd, &epfd);
        }

        if(check_stop_flag() == 1)
            break;
    }
    
    for(int i = 0; i < MAX_CONN; i++)
    {
        close(gateway_main.conn.conns[i].fd);
    }

    for(int i = 0; i < MAX_LISTEN_FD; i++)
    {
        close(gateway_main.conn.listen_fd[i]);
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
        // debug_log("[Data][Thread] Running...");
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
        // debug_log("[Storage][Thread] Running...");
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

    gateway_main.base_port = port;
    gateway_main.http_port = http;

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

void init_sbuffer(void)
{
    sbuffer.head = NULL;
    sbuffer.tail = NULL;

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
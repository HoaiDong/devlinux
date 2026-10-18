#include <stdio.h>
#include "sensor_types.h"
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>   // Các kiểu dữ liệu hệ thống
#include <sys/socket.h>  // socket(), bind(), listen(), accept(), send(), recv()
#include <netinet/in.h>  // struct sockaddr_in, htons(), INADDR_ANY
#include <arpa/inet.h>  // inet_pton()
#include <unistd.h>     // sleep(), close()
#include <signal.h>
#include <time.h>

sensor_data_t sensor;
int stop = 0;

static void debug_log(char *input)
{
    (void)input;
    #ifdef DEBUG
    printf("%s\n", input);
    #endif
}

void check_input_argument(int argc, char *argv[])
{
    if (argc < 6)
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

    int port = atoi(argv[4]);
    
    if (port < 1 || port > 65532)
    {
        printf("[ERROR] Invalid argv\n");
        exit(1);
    }

    char temp[70];
    snprintf(temp, sizeof(temp), "Gateway started (TCP base port %d)", port);
}

static void sigint_hanlder(int sig)
{
    (void)sig;
    stop = 1;
}

static void sigterm_hanlder(int sig)
{
    (void)sig;
    stop = 1;
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

void operation_sensor_temperature(int *fd)
{
    char buffer[18];
    memset(buffer, 0, sizeof(buffer));

    double temperature = (double)(rand() % 1001) / 10.0;

    memcpy(buffer, &sensor.id, 2);
    memcpy(&buffer[2], &temperature, 8);
    memcpy(&buffer[10], &sensor.ts, 8);

    if (send(*fd, buffer, 18, 0) == -1)
    {
        debug_log("[ERROR] Send data error");
    }
}

void get_time()
{
    struct tm time_info;
    sensor.ts = time(NULL);
    localtime_r(&sensor.ts, &time_info);
}

int main(int argc, char *argv[])
{
    int return_value = 0;   
    int period = 0;

    if(argc < 7)
    {
        debug_log("[ERROR] Missing or invalid argc");
        exit(1);
    }

    for(int i = 1; i < argc; i++)
    {
        debug_log(argv[i]);
    }

    init_signal();

    sensor.id = atoi(argv[1]);

    if(strcmp(argv[2], "temperature") == 0)
        sensor.type = SENSOR_TEMPERATURE;
    else if(strcmp(argv[2], "humidity") == 0)
        sensor.type = SENSOR_HUMIDITY;
    else if(strcmp(argv[2], "motion") == 0)
        sensor.type = SENSOR_MOTION;
    else if(strcmp(argv[2], "door") == 0)
        sensor.type = SENSOR_DOOR;
    else if(strcmp(argv[2], "smoke_gas") == 0)
        sensor.type = SENSOR_SMOKE_GAS;
    else
    {
        debug_log("[ERROR] sensor type invalid");
        exit(1);
    }

    

    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd == -1)
    {
        debug_log("[ERROR] create socket unsuccesfull");
        exit(1);
    }

    period = atoi(argv[6]) / 1000;

    int port = atoi(argv[5]);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, argv[4], &addr.sin_addr) != 1)
    {
        debug_log("[ERROR] inet_pton error");
        close(client_fd);
        exit(1);
    }

    return_value = connect(client_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    if(return_value == -1)
    {
        debug_log("[ERROR] connect socket error");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    while(!stop)
    {
        get_time();
        switch (sensor.type)
        {
            case SENSOR_TEMPERATURE:
                operation_sensor_temperature(&client_fd);
                break;
            
            case SENSOR_HUMIDITY:
                /* code */
                break;

            case SENSOR_MOTION:
                /* code */
                break;

            case SENSOR_DOOR:
                /* code */
                break;

            case SENSOR_SMOKE_GAS:
                /* code */
                break;

            default:
                break;
        }
        sleep(period);
    }

    close(client_fd);
    return 0;
}
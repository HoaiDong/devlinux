#include "log_process.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include "data_type.h"
#include <string.h>
#include <signal.h>
#include <pthread.h>

pid_t pid_log_process;
int pipe_fd[2];
int log_fd;
char buffer[PIPE_BUF];
pthread_mutex_t mutex_count = PTHREAD_MUTEX_INITIALIZER;

void log_process_main(void);
void clean_up(void);
void init(void);

static void debug_log(char *input)
{
    (void)input;
    #ifdef DEBUG
    printf("%s\n", input);
    #endif
}

void fork_log_process(void)
{
    fflush(stdout);
    pipe(pipe_fd);

    pid_log_process = fork();

    if (pid_log_process == -1)
    {
        debug_log("[ERROR] fork log process error");
    }
    else if (pid_log_process == 0)
    {
        log_process_main();
    }
    else
    {
        close(pipe_fd[0]);
        char buffer[30];
        snprintf(buffer, sizeof(buffer), "log process PID: %d", pid_log_process);
        debug_log(buffer);
    }
}

void wait_log_process(void)
{
    int exit_status;
    int exit_code;

    debug_log("[MAIN] Kill Log Process");
    char buffer[40];
    snprintf(buffer, sizeof(buffer), "[Log Process] Log process PID: %d", pid_log_process);
    debug_log(buffer);
    // kill(pid_log_process, SIGUSR1);
    debug_log("[MAIN] Wait log process");
    close(pipe_fd[1]);
    waitpid(pid_log_process, &exit_status, 0);

    if(WIFEXITED(exit_status))
    {
        exit_code = WEXITSTATUS(exit_status);

        char buffer[40];
        snprintf(buffer, sizeof(buffer), "[LOG PROCESS] Log process exit code: %d", exit_code);
        debug_log(buffer);
    }
}

int mkfifo(void)
{
    log_fd = open("gateway.log",  O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (log_fd == -1) 
    {
        debug_log("[ERROR] Cannot open file");
        return 1;
    }
    return 0;
}

void send_log(char *source, char *event)
{
    
    char buffer[PIPE_BUF];
    memset(buffer, 0, sizeof(buffer));

    time_t current_time;
    struct tm time_info;
    char time_format[20];
    current_time = time(NULL);
    localtime_r(&current_time, &time_info);
    strftime(time_format, sizeof(time_format), "%Y-%m-%d %H:%M:%S", &time_info);

    pthread_mutex_lock(&mutex_count);
    static int count = 1;
    snprintf(buffer, sizeof(buffer), "%d %s %s %s\n", count, time_format, source, event);
    count++;
    pthread_mutex_unlock(&mutex_count);

    write(pipe_fd[1], buffer, strlen(buffer));
    debug_log(buffer);
    
}

static void sigint_hanlder(int sig)
{
    (void)sig;
    debug_log("[LOG PROCESS] sigint occur");
}

static void sigterm_hanlder(int sig)
{
    (void)sig;
    debug_log("[LOG PROCESS] sigterm occur");
}

static void init_signal(void)
{
    if (signal(SIGINT, sigint_hanlder) != 0)
    {
        exit(1);
    }
    if (signal(SIGTERM, sigterm_hanlder) != 0)
    {
        exit(1);
    }
}

void clean_up(void)
{
    close(pipe_fd[0]);
    close(log_fd);
    pthread_mutex_destroy(&mutex_count);
}

void init(void)
{
    close(pipe_fd[1]);
    memset(buffer, 0, sizeof(buffer));
    debug_log("[LOG PROCESS] Start");
    init_signal();
    pthread_mutex_init(&mutex_count, NULL);
}

void log_process_main(void)
{
    int exit_status = 0;
    init();
    exit_status = mkfifo();

    while(1)
    {
        debug_log("[LOG PROCESS] Waitting read pipe");
        ssize_t byte_read = read(pipe_fd[0], buffer, sizeof(buffer));
        if(byte_read > 0)
        {
            ssize_t byte_write = write(log_fd, buffer, byte_read);
            // debug_log(buffer);
            if(byte_write < 0)
            {
                debug_log("[LOG PROCESS][WRITE LOG] Fail");
            }
            else if (byte_write == 0)
            {
                debug_log("[LOG PROCESS][WRITE LOG] Zero buffer");
            }
            else
            {
                debug_log("[LOG PROCESS][WRITE LOG] OK");
            }
            memset(buffer, 0, sizeof(buffer));
        }
        else if (byte_read == 0)
        {
            debug_log("[LOG PROCESS][WRITE LOG] Stop logging");
            break;
        }
        else 
        {
            debug_log("[LOG PROCESS][WRITE LOG] Error logging");
        }
        
        sleep(DELAY_TIME);
    }

    clean_up();
    debug_log("[LOG PROCESS] End");
    exit(exit_status);
}
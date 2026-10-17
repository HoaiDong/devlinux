#ifndef LOG_PROCESS_H
#define LOG_PROCESS_H


void fork_log_process(void);
void wait_log_process(void);
void send_log(char *source, char *event);
#endif
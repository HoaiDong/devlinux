#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    if (argc < 2) 
    {
        return 2;
    }
    
    pid_t pid = getpid();
    pid_t ppid = getppid();

    printf("[SEARCHER] PID: %d | PPID: %d\n", pid, ppid);
    printf("[SEARCHER] Searching for %s in %s...\n", argv[1], argv[2]);

    FILE *fp = fopen(argv[2], "r");
    if (fp == NULL) {
        return 2;
    }

    char line[256];
    while(fgets(line, sizeof(line), fp) != NULL)
    {
        char *token = strtok(line, "|");
        if(strcmp(token, argv[1]) != 0)
        {
            continue;
        }

        printf("========== SEARCH RESULT ==========\n");
        printf("  ID      : %s\n", token);
        token = strtok(NULL, "|");
        printf("  Name    : %s\n", token);
        token = strtok(NULL, "|");
        printf("  Class   : %s\n", token);
        token = strtok(NULL, "|");
        float gpa = atof(token);
        printf("  GPA     : %0.2f\n", gpa);
        if(gpa >= 8.0)
        {
            printf("  Grade   : Excellent\n");
        }
        else if(gpa >= 7.0)
        {
            printf("  Grade   : Good\n");
        }
        else if(gpa >= 5.0)
        {
            printf("  Grade   : Average\n");
        }
        else
        {
            printf("  Grade   : Poor\n");
        }
        printf("====================================\n");
        fclose(fp);
        return 0;

        
    }

    printf("[SEARCHER] No student found with ID: %s\n", argv[1]);
    fclose(fp);
    return 1;
}
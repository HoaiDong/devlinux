#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

typedef struct {
    int   id;
    char  name[50];
    int   quantity;
    float unit_price;
} Order;

Order orders[3] = {
    {1, "Backpack", 2, 350000},
    {2, "Shoes",    1, 500000},
    {3, "Hat",      3, 120000}
};

void process_order(Order o) {
    float total = o.quantity * o.unit_price;
    printf("[CHILD-%d] PID: %d | PPID: %d\n", o.id, getpid(), getppid());
    printf("[CHILD-%d] %s x%d — Total: %.0f VND\n",
           o.id, o.name, o.quantity, total);
    printf("[CHILD-%d] Processing... (sleep 2s)\n\n", o.id);
    sleep(2);
}

int main()
{
	pid_t pid;   
    int status;
    char student_id[20] = "";

    printf("===================================================\n");
    printf("   STUDENT LOOKUP SYSTEM — MANAGER\n");
    printf("   (fork + execve | file: students.txt)\n");
    printf("===================================================\n");
    printf("[MANAGER] PID: %d\n", getpid());
    printf("Enter student ID ('quit' to exit).\n");

    while(1)
    {
        printf("\n---------------------------------------------\n");
        printf("Student ID: ");
        scanf(" %s", student_id);

        if(strcmp(student_id, "quit") == 0 || strcmp(student_id, "Quit") == 0)
        {
            break;
        }
        else
        {
            fflush(stdout);
            pid = fork();

            if (pid == 0)
            {
                char *argv[] = {"./searcher", student_id, "students.txt", NULL};
                execve("./searcher", argv, NULL);
                /* Only reached if execve() FAILS */
               perror("execve failed"); 
               exit(2);
            }
            else
            {
                printf("[MANAGER] fork() → child PID: %d\n", pid);
            }

            printf("[MANAGER] Waiting for child (waitpid)...\n");
            waitpid(pid, &status, 0);
            if(WIFEXITED(status))
            {
                int exit_code = WEXITSTATUS(status);

                printf("[MANAGER] Child (PID %d) exited. code = %d → ", pid, exit_code);
                switch (exit_code)
                {
                    case 0:
                        printf("Student found\n");
                        break;
                    case 1:
                        printf("Student not found \n");
                        break;
                    case 2:
                        printf("File or argument error \n");
                        break;
                    default:
                        break;
                }
            }
        }
    } 

    printf("[MANAGER] Exiting. Goodbye!\n");
	return 0;
}
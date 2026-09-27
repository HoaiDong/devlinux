#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

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
	pid_t pid[3];   
    int status[3];
    int count = 0;

    printf("===================================================\n");
    printf("ORDER PROCESSING SYSTEM — MANAGER (fork+wait)\n");
    printf("===================================================\n");
    printf("[MANAGER] PID: %d — spawning 3 child processes...\n", getpid());

	for (int i = 0; i < 3; i++)
	{
        fflush(stdout);
		pid[i] = fork();

		if (pid[i] == 0)
		{
			process_order(orders[i]);
			exit(0);
		}
        else
        {
            printf("[MANAGER] fork() order #%d → child PID: %d\n", i+1, pid[i]);

        }
	}
	
    printf("[MANAGER] All 3 children spawned. Starting waitpid()...\n");
    printf("--- [child output order may interleave — this is normal] ---\n");
    printf("--- [~2 seconds later, all 3 children call exit(0)] ---\n");

    for (int i = 0; i < 3; i++)
    {
        printf("[MANAGER] waitpid(%d) — order #%d:", pid[i], i+1);
        waitpid(pid[i], &status[i], 0);
        if(WIFEXITED(status[i]))
        {
            int exit_code = WEXITSTATUS(status[i]);
            printf(" exit code = %d → SUCCESS\n", exit_code);
            count++;
        }
    }

    printf("================= SUMMARY =================\n");
    printf("  Total orders    : 3\n");
    printf("  Successful      : %d\n", count);
    printf("  Failed          : %d\n", 3 - count);
    printf("  Total revenue   : 1,560,000 VND\n");
    printf("===========================================\n");
	return 0;
}
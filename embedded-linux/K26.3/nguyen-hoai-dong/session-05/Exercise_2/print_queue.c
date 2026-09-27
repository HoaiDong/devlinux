#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define QUEUE_SIZE 5
typedef struct {
    int  doc_id;
    char filename[60];
    int  pages;
} Document;

Document queue[QUEUE_SIZE];
int head = 0, tail = 0, count = 0;
int all_sent = 0;           /* set to 1 by main after joining all producers */

pthread_mutex_t q_lock;
pthread_cond_t  not_full;   /* producers wait here when count == 5 */
pthread_cond_t  not_empty;  /* printer  waits here when count == 0 */

void enqueue(Document *doc)
{
    queue[head] = *doc;
    head = (head + 1)%QUEUE_SIZE;
    count++;
}

void dequeue(Document *doc)
{
    *doc = queue[tail];
    tail = (tail + 1)%QUEUE_SIZE;
    count--;
}

void *producer(void *arg)
{
    if(arg == NULL)
    {
        return NULL;
    }

    Document *doc = (Document*) arg;


    for(int i = 0; i < 3; i++)
    {
        pthread_mutex_lock(&q_lock);
        while (count == 5)
        {
            printf("[Producer %d] Queue full — waiting...\n", (doc+i)->doc_id);
            pthread_cond_wait(&not_full, &q_lock);  //sleep if queue is full
        }
        enqueue(doc+i);
        printf("[Producer %d] Submitting: %s (%d pages) — queue: %d/5\n",
                (doc+i)->doc_id,
                (doc+i)->filename,
                (doc+i)->pages,
                count
            );
        pthread_cond_signal(&not_empty);    //wake the printer
        pthread_mutex_unlock(&q_lock);
    }
    
}

void *printer(void *arg)
{
    Document doc;

    while (1)
    {
        pthread_mutex_lock(&q_lock);
        while (count == 0 && !all_sent)
        {
            //sleep if queue is empty
            pthread_cond_wait(&not_empty, &q_lock);   
        }
        
        //unlock → break
        if (count == 0 && all_sent)
        {   
            printf("[Printer]    All documents printed. Exiting.\n");
            return NULL;
        }

        dequeue(&doc);
        printf("[Printer]    Printing:   %s   (%d pages)  — queue: %d/5\n",
                doc.filename,
                doc.pages,
                count
            );

        pthread_cond_signal(&not_full); //wake a waiting producer
        pthread_mutex_unlock(&q_lock);
        sleep(1);   //simulate printing time
    }
}

int main()
{
    pthread_t producer_thread[3];
    pthread_t printer_thread;
    Document producer_1_doc[3] = {{1, "report_Q1.pdf", 12}, {1, "slides.pdf", 20}, {1, "summary.pdf", 4}};
    Document producer_2_doc[3] = {{2, "contract.pdf", 5}, {2, "memo.pdf", 2}, {2, "budget.pdf", 7}};
    Document producer_3_doc[3] = {{3, "invoice.pdf", 3}, {3, "proposal.pdf", 8}, {3, "another.pdf", 10}};

    pthread_mutex_init(&q_lock, NULL);

    printf("==============================================\n");
    printf("   OFFICE PRINT QUEUE (3 producers, 1 printer)\n");
    printf("   Queue capacity: 5 documents\n");
    printf("==============================================\n\n");


    pthread_create(&producer_thread[0], NULL, producer, producer_1_doc);
    pthread_create(&producer_thread[1], NULL, producer, producer_2_doc);
    pthread_create(&producer_thread[2], NULL, producer, producer_3_doc);
    pthread_create(&printer_thread, NULL, printer, NULL);

    for(int i = 0; i < 3; i++)
    {
        pthread_join(producer_thread[i], NULL);
    }
    all_sent = 1;
    pthread_join(printer_thread, NULL);

    printf("\n================ SUMMARY ================\n");
    printf("  Documents submitted : 9\n");
    printf("  Documents printed   : 9\n");
    printf("  Total pages printed : 66\n");
    printf("==============================================\n\n");

    pthread_mutex_destroy(&q_lock);
    return 0;
}
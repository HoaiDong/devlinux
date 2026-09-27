#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define NUMBER_AGENT 5
#define TOTAL_SEAT 10

typedef struct {
    int  agent_id;
    char customer[50];
    int  seats_wanted;
} BookingRequest;

BookingRequest requests[5] = {
    {1, "Nguyen Van An",  2},
    {2, "Tran Thi Bich",  1},
    {3, "Le Van Cuong",   3},
    {4, "Pham Thi Dung",  1},
    {5, "Hoang Van Em",   2}
};

int seats_available = TOTAL_SEAT;
pthread_mutex_t seat_lock;

void *book_ticket(void *arg)
{
    if(arg == NULL)
    {
        return NULL;
    }

    BookingRequest *booking_request = (BookingRequest*)arg;

    printf("[Agent %d | TID %ld...] Booking %d seats for %s...\n", 
            booking_request->agent_id,
            pthread_self(),
            booking_request->seats_wanted,
            booking_request->customer
        );

    sleep(1);
    pthread_mutex_lock(&seat_lock);
    if(seats_available >= booking_request->seats_wanted)
    {
        seats_available -= booking_request->seats_wanted;
        printf("[Agent %d] CONFIRMED: %d seats for %s.  Remaining: %d\n", 
            booking_request->agent_id,
            booking_request->seats_wanted,
            booking_request->customer,
            seats_available
        );
    }
    else
    {
        printf("[Agent %d] SOLD OUT:  needs %d seats, only %d left — booking failed.\n", 
            booking_request->agent_id,
            booking_request->seats_wanted,
            seats_available
        );
    }
    pthread_mutex_unlock(&seat_lock);
}

int main()
{
    pthread_t pthread[5];
    pthread_mutex_init(&seat_lock, NULL);

    printf("==============================================\n");
    printf("   TICKET BOOKING SYSTEM (%d agents, %d seats)\n", NUMBER_AGENT, seats_available);
    printf("==============================================\n\n");

    for(int i = 0; i < NUMBER_AGENT; i++)
    {
        pthread_create(&pthread[i], NULL, book_ticket, &requests[i]);
    }

    printf("--- [all agents reach critical section after sleep(1)] ---\n");

    for(int i = 0; i < NUMBER_AGENT; i++)
    {
        pthread_join(pthread[i], NULL);
    }

    printf("\n================ SUMMARY ================\n");
    printf("  Total seats     : %d\n", TOTAL_SEAT);
    printf("  Seats sold      : %d\n", TOTAL_SEAT - seats_available);
    printf("  Seats remaining : %d\n", seats_available);
    printf("  Failed bookings : %d\n", 1);
    printf("=========================================\n");

    pthread_mutex_destroy(&seat_lock);
    return 0;
}
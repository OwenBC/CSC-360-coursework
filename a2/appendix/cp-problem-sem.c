#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

/* Producer/consumer program illustrating semaphores */
/* Adapted from chapter 30 of Operating Systems: Three Easy Pieces at https://pages.cs.wisc.edu/~remzi/OSTEP/ */

/* Size of shared buffer */
#define BUF_SIZE 3

int buffer[BUF_SIZE];  /* shared buffer */
int add = 0;           /* place to add next element */
int rem = 0;           /* place to remove next element */
int num = 0;           /* number elements in buffer */
sem_t full;
sem_t empty;

void *producer(void *param);
void *consumer(void *param);

int main(int argc, char *argv[])
{
    pthread_t tid1, tid2; /* thread identifiers */

    sem_init(&empty, 0, BUF_SIZE);
    sem_init(&full, 0, 0);

    /* create the threads; may be any number, in general */
    if (pthread_create(&tid1, NULL, producer, NULL) != 0)
    {
        fprintf(stderr, "Unable to create producer thread\n");
        exit(1);
    }
    if (pthread_create(&tid2, NULL, consumer, NULL) != 0)
    {
        fprintf(stderr, "Unable to create consumer thread\n");
        exit(1);
    }
    /* wait for created thread to exit */
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    sem_destroy(&empty);
    sem_destroy(&full);
    printf("Parent quitting\n");
    return 0;
}

/* Produce value(s) */
void *producer(void *param)
{
    int i;
    for (i = 1; i <= 20; i++)
    {
        /* Insert into buffer */
        sem_wait(&empty);
        if (num > BUF_SIZE)
            exit(1);            /* overflow */
        buffer[add] = i;
        add = (add + 1) % BUF_SIZE;
        num++;
        sem_post(&full);
        printf("producer: inserted %d\n", i);
        fflush(stdout);
    }
    printf("producer quitting\n");
    fflush(stdout);
    pthread_exit(NULL);
}

void *consumer(void *param)
{
    int i;
    for (i = 1; i <= 20; i++)
    {
        sem_wait(&full);
        i = buffer[rem];
        rem = (rem + 1) % BUF_SIZE;
        num--;
        sem_post(&empty);
        printf("Consume value %d\n", i);
        fflush(stdout);
    }
    pthread_exit(NULL);
}
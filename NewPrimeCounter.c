#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>
#include <time.h>

#define MAX_NUM 1000000    // Adjust based on your input size
#define MAX_QUEUE_SIZE 512 // Adjust to fit memory constraints

// Atomic counter to store the total number of primes found
atomic_int total_counter = 0;

// Function to check if a number is prime
bool isPrime(int n)
{
    if (n <= 1)
        return false;
    if (n <= 3)
        return true;
    if (n % 2 == 0 || n % 3 == 0)
        return false;
    for (int i = 5; i * i <= n; i += 6)
    {
        if (n % i == 0 || n % (i + 2) == 0)
            return false;
    }
    return true;
}

// Queue structure
typedef struct
{
    int values[MAX_QUEUE_SIZE];
    int head;
    int tail;
    atomic_flag lock;
} Queue;

// Initialize the queue
void initQueue(Queue *q)
{
    q->head = q->tail = 0;
    atomic_flag_clear(&q->lock);
}

// Enqueue an element into the queue
void enqueue(Queue *q, int value)
{
    while (atomic_flag_test_and_set(&q->lock))
    {
    }
    q->values[q->tail] = value;
    q->tail = (q->tail + 1) % MAX_QUEUE_SIZE;
    atomic_flag_clear(&q->lock);
}

// Dequeue an element from the queue
bool dequeue(Queue *q, int *value)
{
    while (atomic_flag_test_and_set(&q->lock))
    {
    }
    if (q->head == q->tail)
    {
        atomic_flag_clear(&q->lock);
        return false;
    }
    *value = q->values[q->head];
    q->head = (q->head + 1) % MAX_QUEUE_SIZE;
    atomic_flag_clear(&q->lock);
    return true;
}

// Function for a thread to process numbers from the queue
void *processQueue(void *arg)
{
    Queue *queue = (Queue *)arg;
    int num;

    while (1)
    {
        if (dequeue(queue, &num))
        {
            if (isPrime(num))
            {
                atomic_fetch_add(&total_counter, 1);
            }
        }
        else
        {
            break; // Exit when queue is empty
        }
    }
    return NULL;
}

int main()
{
    int num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    pthread_t threads[num_threads];
    Queue queues[num_threads];

    int num;

    // Initialize queues
    for (int i = 0; i < num_threads; i++)
    {
        initQueue(&queues[i]);
    }

    struct timespec start, end;
    if (clock_gettime(CLOCK_REALTIME, &start) == -1)
    {
        perror("clock_gettime");
        return 1;
    }

    // Read numbers from stdin and distribute them to the queues
    int index = 0;
    while (scanf("%d", &num) != EOF)
    {
        enqueue(&queues[index], num);
        index = (index + 1) % num_threads;
    }

    // Spawn threads to process numbers from the queue
    for (int i = 0; i < num_threads; i++)
    {
        pthread_create(&threads[i], NULL, processQueue, &queues[i]);
    }

    // Join all threads
    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    if (clock_gettime(CLOCK_REALTIME, &end) == -1)
    {
        perror("clock_gettime");
        return 1;
    }

    long seconds = end.tv_sec - start.tv_sec;
    long nanoseconds = end.tv_nsec - start.tv_nsec;
    if (nanoseconds < 0)
    {
        seconds -= 1;
        nanoseconds += 1000000000L;
    }

    printf("runing time: %ld.%09ld seconds\n", seconds, nanoseconds);
    printf("%d total primes.\n", atomic_load(&total_counter));

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

struct shared
{
    pthread_mutex_t *mutex;
    unsigned int task_id;
};

#define SHARED_MEMORY_NAME getenv("SRSV_LAB5")
#define SHARED_MEMORY_SIZE sizeof(struct shared)
#define MSG_MAXMSGS 10
#define MSG_MAXMSGSZ 256

struct task_descriptor
{
    unsigned int task_id;
    unsigned int task_duration;
    char shared_memory_name[128];
};

struct args
{
    unsigned int N, M;
};

// needed for synchronisation
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// FIFO list implementation
struct node
{
    struct task_descriptor *data;
    struct node *next;
};

struct queue
{
    struct node *head;
    struct node *tail;
    unsigned int N;
    unsigned int M;
};

struct queue queue;

void enqueue(struct queue *queue, struct task_descriptor *element)
{
    pthread_mutex_lock(&mutex);
    struct node *new = (struct node *)malloc(sizeof(struct node));

    new->data = element;
    new->next = NULL;
    queue->N += 1;
    queue->M += element->task_duration;

    if (queue->head == NULL)
    {
        queue->head = queue->tail = new;
    }
    else
    {
        queue->tail->next = new;
        queue->tail = new;
    }
    pthread_mutex_unlock(&mutex);
}

int is_empty(struct queue *queue)
{
    if (queue->head == NULL)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

struct task_descriptor *dequeue(struct queue *queue)
{
    pthread_mutex_lock(&mutex);
    while (is_empty(queue))
    {
        pthread_cond_wait(&cond, &mutex);
    }
    if (is_empty(queue))
    {
        return NULL;
    }

    struct task_descriptor *element = queue->head->data;
    queue->N -= 1;
    queue->M -= element->task_duration;

    struct node *tmp = (struct node *)queue->head;
    if (queue->head == queue->tail)
    {
        queue->head = queue->tail = NULL;
    }
    else
    {
        queue->head = tmp->next;
    }

    free(tmp);
    pthread_mutex_unlock(&mutex);

    return element;
}

int num_of_msgs = 0, durations_sum = 0;

void sig_handler(int signum)
{
    if (is_empty(&queue) != 1)
    {
        printf("P: pokrecem zaostale poslove (nakon isteka vise od 30 sekundi)\n");
        pthread_cond_broadcast(&cond);
        num_of_msgs = durations_sum = 0;
    }
}

void *reciever(void *args)
{
    struct args arguments = *(struct args *)args;

    // making the shared memory name string
    char name_buffer[128];
    strcpy(name_buffer, "/");
    strcat(name_buffer, SHARED_MEMORY_NAME);

    // opening a message queue
    mqd_t mqdes;
    struct mq_attr attr = {.mq_curmsgs = 0, .mq_flags = 0, .mq_maxmsg = MSG_MAXMSGS, .mq_msgsize = MSG_MAXMSGSZ};
    unsigned msg_prio = 10;
    mqdes = mq_open(name_buffer, O_RDONLY | O_CREAT, 00600, &attr);
    if (mqdes == (mqd_t)-1)
    {
        perror("server:mq_open");
        exit(1);
    }
    else
    {
        char message[MSG_MAXMSGSZ];
        signal(SIGALRM, sig_handler); // Register signal handler
        while (1)
        {
            while (num_of_msgs < arguments.N || durations_sum < arguments.M)
            {
                int msg_len = mq_receive(mqdes, message, MSG_MAXMSGSZ, &msg_prio);
                if (msg_len < 0)
                {
                    // just wait for a message, dont raise exceptions
                }
                else
                {
                    alarm(30); // Scheduled alarm after 30 seconds
                    num_of_msgs++;
                    int id, duration;
                    char name[128];
                    sscanf(message, "%d %d %s", &id, &duration, name);
                    durations_sum += duration;
                    printf("P: zaprimio %d %d %s\n", id, duration, name);

                    struct task_descriptor *desc = (struct task_descriptor *)malloc(sizeof(struct task_descriptor));
                    desc->task_id = id;
                    desc->task_duration = duration;
                    strcpy(desc->shared_memory_name, name);
                    enqueue(&queue, desc);
                }
            }
            num_of_msgs = durations_sum = 0;
            pthread_cond_broadcast(&cond);
        }
    }
}

void loop(unsigned long long int iterations)
{
    while (iterations)
    {
        asm volatile("" ::
                         : "memory");
        iterations--;
    }
}

void *worker(void *args)
{
    unsigned long long int *iterations = (unsigned long long int *)args;

    struct task_descriptor *t_d;
    while (1)
    {
        t_d = dequeue(&queue);

        int shared_memory_id;

        // writing to shared memory
        shared_memory_id = shm_open(t_d->shared_memory_name, O_RDWR, 00600);

        struct task_description
        {
            struct task_descriptor t_d;
            int data_array[t_d->task_duration];
        };

        if (shared_memory_id == -1 || ftruncate(shared_memory_id, sizeof(struct task_description)) == -1)
        {
            perror("server:shm_open/ftruncate");
            exit(1);
        }

        struct task_description *description;
        description = mmap(NULL, sizeof(struct task_description), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_id, 0);
        if (description == (void *)-1)
        {
            perror("server:mmap");
            exit(1);
        }

        for (int i = 0; i < t_d->task_duration; i++)
        {
            printf("R%d: id:%d obrada podatka: %d (%d/%d)\n", t_d->task_id, t_d->task_id, description->data_array[i], i + 1, t_d->task_duration);
            loop(*iterations);
        }
        printf("R%d: id:%d obrada gotova\n", t_d->task_id, t_d->task_id);
    }
}

int main(int argc, char *argv[])
{
    struct args arguments;

    // number of worker threads
    arguments.N = atoi(argv[1]);

    // mainimum sum of task durations
    arguments.M = atoi(argv[2]);

    queue.head = NULL;
    queue.tail = NULL;
    queue.N = arguments.N;
    queue.M = arguments.M;

    // finding number of iterations needed for one second on this cpu
    unsigned long long int iterations = 1000000;
    double time_elapsed = 0;
    struct timespec start, stop;
    while ((time_elapsed / 1e6) < 1)
    {
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
        loop(iterations);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop);
        time_elapsed = (stop.tv_sec - start.tv_sec) * 1e6 + (stop.tv_nsec - start.tv_nsec) / 1e3;
        iterations *= 1.1;
    }

    printf("Found the number of iterations needed for one second: %llu\n", iterations);

    pthread_t reciever_thread;

    if (pthread_create(&reciever_thread, NULL, reciever, (void *)&arguments) != 0)
    {
        perror("ERROR: pthread_create:");
        return -1;
    }

    pthread_t worker_threads[arguments.N];

    for (int i = 0; i < arguments.N; i++)
    {
        if (pthread_create(&worker_threads[i], NULL, worker, (void *)&iterations) != 0)
        {
            perror("ERROR: pthread_create:");
            return -1;
        }
    }

    // wait until reciever thread finishes
    pthread_join(reciever_thread, NULL);

    // wait until all worker thread finish
    for (int i = 0; i < arguments.N; i++)
    {
        pthread_join(worker_threads[i], NULL);
    }

    printf("Server gotov\n");

    return 0;
}
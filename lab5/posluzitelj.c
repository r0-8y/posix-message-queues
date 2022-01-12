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
#define MSG_MAXMSGS  10
#define MSG_MAXMSGSZ 256

struct task_descriptor
{
    unsigned int task_id;
    unsigned int task_duration;
    const char *shared_memory_name;
};

struct args
{
    unsigned int N, M;
};

void *server(void *args)
{
    // struct args arguments = *(struct args *)args;

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
        printf("Server opened the queue\n");
    }

    char message[MSG_MAXMSGSZ];
    while (1)
    {
        int msg_len = mq_receive(mqdes, message, MSG_MAXMSGSZ, &msg_prio);
        if (msg_len < 0)
        {
            // perror("mq_receive");
            // exit(1);
        } else {
            int id, duration;
            char *name;
            sscanf(message, "%d %d %s", &id, &duration, name); 
            printf("Task id: %d\n", id);
            printf("Task duration: %d\n", duration);
            printf("Shared memory segment: %s\n", name);
        }
    }
}

int main(int argc, char *argv[])
{
    struct args arguments;

    // number of worker threads
    arguments.N = atoi(argv[1]);

    // mainimum sum of task durations
    arguments.M = atoi(argv[2]);

    pthread_t server_thread;

    if (pthread_create(&server_thread, NULL, server, (void *)&arguments) != 0)
    {
        perror("ERROR: pthread_create:");
        return -1;
    }

    // wait until created thread finishes
    pthread_join(server_thread, NULL);

    printf("Server done\n");

    return 0;
}
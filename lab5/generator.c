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

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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
    int J, K;
};

void *generator(void *args)
{
    struct args arguments = *(struct args *)args;

    // initial task id
    int task_id = 0;

    // shared memory id
    int shared_memory_id;

    // shared memory pointer
    struct shared *pointer;

    // making the shared memory name string
    char name_buffer[128];
    strcpy(name_buffer, "/");
    strcat(name_buffer, SHARED_MEMORY_NAME);

    // retrieving the shared memory segment
    shared_memory_id = shm_open(name_buffer, O_CREAT | O_RDWR, 00600);
    if (shared_memory_id == -1 || ftruncate(shared_memory_id, SHARED_MEMORY_SIZE) == -1)
    {
        perror("generator:shm_open/ftruncate");
        exit(1);
    }

    pointer = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_id, 0);
    if (pointer == (void *)-1)
    {
        perror("generator:mmap");
        exit(1);
    }

    pointer->mutex = &mutex;

    // locking the mutex for safe id reservation
    pthread_mutex_lock(pointer->mutex);
    if (pointer->task_id)
    {
        task_id = pointer->task_id;
        pointer->task_id = pointer->task_id + arguments.J;
    }
    else
    {
        pointer->task_id = arguments.J;
    }
    pthread_mutex_unlock(pointer->mutex);
    munmap(pointer, SHARED_MEMORY_SIZE);
    close(shared_memory_id);

    // opening a message queue
    mqd_t mqdes;
    struct mq_attr attr = {.mq_curmsgs = 0, .mq_flags = 0, .mq_maxmsg = MSG_MAXMSGS, .mq_msgsize = MSG_MAXMSGSZ};
    unsigned msg_prio = 10;
    mqdes = mq_open(name_buffer, O_WRONLY | O_CREAT, 00600, &attr);
    if (mqdes == (mqd_t)-1)
    {
        perror("generator:mq_open");
        exit(1);
    }

    // generating J tasks of maximum K duration
    struct task_descriptor desc;
    srand(time(NULL));
    for (int id = task_id; id < task_id + arguments.J; id++)
    {
        // setting the descriptor id
        desc.task_id = id;

        // setting task duration
        desc.task_duration = random() % arguments.K + 1;

        // making the shared memory name string for every task
        char id_buffer[10];
        strcpy(desc.shared_memory_name, name_buffer);
        strcat(desc.shared_memory_name, "-");
        snprintf(id_buffer, 10, "%d", id);
        strcat(desc.shared_memory_name, id_buffer);

        // writing to shared memory
        shared_memory_id = shm_open(desc.shared_memory_name, O_CREAT | O_RDWR, 00600);
        struct task_description
        {
            struct task_descriptor t_d;
            int data_array[desc.task_duration];
        };
        if (shared_memory_id == -1 || ftruncate(shared_memory_id, sizeof(struct task_description)) == -1)
        {
            perror("generator:shm_open/ftruncate");
            exit(1);
        }

        struct task_description *description;
        description = mmap(NULL, sizeof(struct task_description), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_id, 0);
        if (description == (void *)-1)
        {
            perror("generator:mmap");
            exit(1);
        }

        description->t_d = desc;
        for (int i = 0; i < desc.task_duration; i++)
        {
            description->data_array[i] = random() % 900 + 100;
        }

        char message[MSG_MAXMSGSZ];
        sprintf(message, "%d %d %s", desc.task_id, desc.task_duration, desc.shared_memory_name);
        if (mq_send(mqdes, message, MSG_MAXMSGSZ, msg_prio) == -1)
        {
            perror("generator:mq_send");
            exit(1);
        }
        else
        {
            printf("G: posao %d %d %s [ ", desc.task_id, desc.task_duration, desc.shared_memory_name);
            for (int i = 0; i < desc.task_duration; i++)
            {
                printf("%d ", description->data_array[i]);
            }
            printf("]\n");
            sleep(1);
        }
    }
}

int main(int argc, char *argv[])
{
    struct args arguments;

    // number of tasks
    arguments.J = atoi(argv[1]);

    // maksimum task duration
    arguments.K = atoi(argv[2]);

    pthread_t generator_thread;

    if (pthread_create(&generator_thread, NULL, generator, (void *)&arguments) != 0)
    {
        perror("ERROR: pthread_create:");
        return -1;
    }

    // wait until created thread finishes
    pthread_join(generator_thread, NULL);

    printf("Generator gotov\n");

    return 0;
}
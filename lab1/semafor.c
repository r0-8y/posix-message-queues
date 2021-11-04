#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>

#define MSG_SIZE 32
#define MSG_TYPE_CAME 1
#define MSG_TYPE_SET_SEM 2
#define MSG_TYPE_GO_H 4
#define MSG_TYPE_GO_V 5
#define MSG_TYPE_H 6
#define MSG_TYPE_V 7
#define Q_UPR_SEM 3

struct my_msgbuf
{
    long mtype;
    int mtext[MSG_SIZE];
};

struct position
{
    int x, y;
};

int main(void)
{
    struct my_msgbuf buf_sem, buf_pos;
    int id_sem, time, time_counter, direction;
    key_t key_sem = Q_UPR_SEM;

    if ((id_sem = msgget(key_sem, IPC_CREAT | 0666)) == -1)
    {
        perror("msgget");
        exit(1);
    }

    while (1)
    {
        if ((msgrcv(id_sem, (struct msgbuf *)&buf_sem, MSG_SIZE, MSG_TYPE_SET_SEM, 0)) == -1)
        {
            perror("msgrcv");
            exit(1);
        }
        else
        {
            printf("Trenutni smijer: %d. (1=hor, -1=ver)\n", buf_sem.mtext[0]);
            time = buf_sem.mtext[1];
            time_counter = 0;
            direction = buf_sem.mtext[0];

            while (--time)
            {
                for (int i = 0; i < 5; i++)
                {
                    if ((msgrcv(id_sem, (struct msgbuf *)&buf_sem, MSG_SIZE, direction > 0 ? MSG_TYPE_H : MSG_TYPE_V, IPC_NOWAIT)) == -1)
                    {
                    }
                    else
                    {
                        //time_counter++;
                        buf_pos.mtype = direction > 0 ? MSG_TYPE_GO_H : MSG_TYPE_GO_V;
                        if ((msgsnd(id_sem, (struct msgbuf *)&buf_pos, MSG_SIZE, 0)) == -1)
                        {
                            perror("msgsnd");
                        }
                    }
                }
                /* printf("Pustio sam %d sudionika u smijeru %d.\n", time_counter, direction);
                time_counter = 0; */
                sleep(1);
            }
        }
    }

    return 0;
}

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
#define MSG_TYPE_H 6
#define MSG_TYPE_V 7
#define Q_POS_UPR 2
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
    struct my_msgbuf buf_pos, buf_sem;
    int id_upr, id_sem;
    key_t key_upr = Q_POS_UPR, key_sem = Q_UPR_SEM;
    int t = 10;
    // 1 = horizontal, -1 = vertical
    int direction = 1 /* , waiting = -1 */;

    if ((id_upr = msgget(key_upr, IPC_CREAT | 0666)) == -1)
    {
        perror("msgget");
        exit(1);
    }

    if ((id_sem = msgget(key_sem, 0666)) == -1)
    {
        perror("msgget");
        exit(1);
    }

    while (1)
    {
        if (t == 10 /* 5 || t == 30 */)
        {
            buf_sem.mtype = MSG_TYPE_SET_SEM;
            buf_sem.mtext[0] = direction;
            buf_sem.mtext[1] = t;
            printf("%ld: Trenutni smijer: %d. (1=hor, -1=ver)\n", time(NULL), direction);
            if ((msgsnd(id_sem, (struct msgbuf *)&buf_sem, MSG_SIZE, 0)) == -1)
            {
                perror("msgsnd");
            }
        }

        sleep(1);

        if ((msgrcv(id_upr, (struct msgbuf *)&buf_pos, MSG_SIZE, MSG_TYPE_CAME, IPC_NOWAIT)) == -1)
        {
            //printf("Nitko novi nije došao.\n");
        }
        else
        {
            // izracunavanje smijera sudionika
            buf_sem.mtext[0] = buf_pos.mtext[0]; // saljem info je li auto ili pjesak
            if (buf_pos.mtext[0])
            {
                if (buf_pos.mtext[1] % 2 != 0)
                {
                    buf_sem.mtype = MSG_TYPE_H;
                }
                else
                {
                    buf_sem.mtype = MSG_TYPE_V;
                }
            }
            else
            {
                if (buf_pos.mtext[1] < 4)
                {
                    buf_sem.mtype = MSG_TYPE_H;
                }
                else
                {
                    buf_sem.mtype = MSG_TYPE_V;
                }
            }

            // javljam semaforu u kojem smijeru je sudionik dosao
            if ((msgsnd(id_sem, (struct msgbuf *)&buf_sem, MSG_SIZE, 0)) == -1)
            {
                perror("msgsnd");
            }
        }

        if (--t == 0)
        {
            t = 10; /* waiting > 0 ? 30 : 15; */
            //waiting = -1;
            direction *= -1;
            // cekamo izmedu ciklusa da svi produ kroz raskrizje
            sleep(5);
        }
    }

    return 0;
}

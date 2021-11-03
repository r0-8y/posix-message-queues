#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>

#define MSG_SIZE 32
#define MSG_TYPE_CAME 1
#define MSG_TYPE_LEFT 2
#define MSG_TYPE_PASSING 3

#define Q_POS_RAS 1

struct my_msgbuf
{
    long mtype;
    int mtext[MSG_SIZE];
};

struct position
{
    int x, y;
};

struct position pedestrians[8] = {{1, 8}, {9, 8}, {9, 2}, {1, 2}, {2, 9}, {8, 9}, {8, 1}, {2, 1}};
struct position cars[4] = {{4, 8}, {8, 6}, {6, 2}, {2, 4}};
char intersection[11][11] = {' ', ' ', ' ', '|', ' ', '|', ' ', '|', ' ', ' ', ' ',
                             ' ', ' ', ' ', '|', ' ', '|', ' ', '|', ' ', ' ', ' ',
                             ' ', ' ', ' ', '|', ' ', '|', ' ', '|', ' ', ' ', ' ',
                             '-', '-', '-', '+', ' ', ' ', ' ', '+', '-', '-', '-',
                             ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                             '-', '-', '-', ' ', ' ', ' ', ' ', ' ', '-', '-', '-',
                             ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                             '-', '-', '-', '+', ' ', ' ', ' ', '+', '-', '-', '-',
                             ' ', ' ', ' ', '|', ' ', '|', ' ', '|', ' ', ' ', ' ',
                             ' ', ' ', ' ', '|', ' ', '|', ' ', '|', ' ', ' ', ' ',
                             ' ', ' ', ' ', '|', ' ', '|', ' ', '|', ' ', ' ', ' '};

void print_state()
{
    for (int i = 0; i < 11; i++)
    {
        for (int j = 0; j < 11; j++)
        {
            printf("%c", intersection[i][j]);
        }
        printf("\n");
    }

    printf("\n");
}

int main(void)
{
    struct my_msgbuf buf;
    int id_ras;
    key_t key_ras = Q_POS_RAS;

    if ((id_ras = msgget(key_ras, IPC_CREAT | 0666)) == -1)
    {
        perror("msgget");
        exit(1);
    }

    while (1)
    {
        print_state();
        if ((msgrcv(id_ras, (struct msgbuf *)&buf, MSG_SIZE, MSG_TYPE_CAME, IPC_NOWAIT)) == -1)
        {
        }
        else
        {
            if (buf.mtext[0])
            {
                intersection[cars[buf.mtext[1]].x][cars[buf.mtext[1]].y] = 'A';
            }
            else
            {
                intersection[pedestrians[buf.mtext[1]].x][pedestrians[buf.mtext[1]].y] = 'p';
            }
        }

        if ((msgrcv(id_ras, (struct msgbuf *)&buf, MSG_SIZE, MSG_TYPE_PASSING, IPC_NOWAIT)) == -1)
        {
        }
        else
        {
            if (buf.mtext[0])
            {
                intersection[cars[buf.mtext[1]].x][cars[buf.mtext[1]].y] = ' ';
                if (buf.mtext[1] % 2 == 0)
                {
                    for (int i = 0; i < 11; i++)
                    {
                        intersection[cars[buf.mtext[1]].x][i] = 'A';
                    }
                }
                else
                {
                    for (int i = 0; i < 11; i++)
                    {
                        intersection[i][cars[buf.mtext[1]].y] = 'A';
                    }
                }
            }
            else
            {
                intersection[pedestrians[buf.mtext[1]].x][pedestrians[buf.mtext[1]].y] = ' ';
                if (buf.mtext[1] < 4)
                {
                    intersection[pedestrians[buf.mtext[1]].x][4] = 'p';
                    intersection[pedestrians[buf.mtext[1]].x][6] = 'p';
                }
                else
                {
                    intersection[4][pedestrians[buf.mtext[1]].y] = 'p';
                    intersection[6][pedestrians[buf.mtext[1]].y] = 'p';
                }
            }
        }

        if ((msgrcv(id_ras, (struct msgbuf *)&buf, MSG_SIZE, MSG_TYPE_LEFT, IPC_NOWAIT)) == -1)
        {
        }
        else
        {
            if (buf.mtext[0])
            {
                if (buf.mtext[1] % 2 == 0)
                {
                    for (int i = 0; i < 11; i++)
                    {
                        intersection[cars[buf.mtext[1]].x][i] = ' ';
                    }
                }
                else
                {
                    for (int i = 0; i < 11; i++)
                    {
                        intersection[i][cars[buf.mtext[1]].y] = ' ';
                    }
                }
            }
            else
            {
                if (buf.mtext[1] < 4)
                {
                    intersection[pedestrians[buf.mtext[1]].x][4] = ' ';
                    intersection[pedestrians[buf.mtext[1]].x][6] = ' ';
                }
                else
                {
                    intersection[4][pedestrians[buf.mtext[1]].y] = ' ';
                    intersection[6][pedestrians[buf.mtext[1]].y] = ' ';
                }
            }
        }
        sleep(1);
    }

    return 0;
}

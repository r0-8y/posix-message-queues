#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <wait.h>

#define MSG_SIZE 32
#define MSG_TYPE_CAME 1
#define MSG_TYPE_PASSING 3
#define MSG_TYPE_LEFT 2
#define MSG_TYPE_GO_H 4
#define MSG_TYPE_GO_V 5

#define Q_POS_RAS 1 // kljuc za red poruka s ras
#define Q_POS_UPR 2 // kljuc za red poruka s upr
#define Q_POS_SEM 3 // kljuc za red poruka sa sem
#define CAR 1

struct my_msgbuf
{
    long mtype;
    int mtext[MSG_SIZE];
};

int main(void)
{
    srand(time(NULL));
    int id = 0;
    struct my_msgbuf buf;
    int id_ras, id_upr, id_sem;
    key_t key_ras = Q_POS_RAS, key_upr = Q_POS_UPR, key_sem = Q_POS_SEM;

    if ((id_ras = msgget(key_ras, 0666)) == -1)
    {
        perror("msgget");
        exit(1);
    }

    if ((id_upr = msgget(key_upr, 0666)) == -1)
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
        sleep(rand() % 5 + 10);

        switch (fork())
        {

        case -1:
            printf("Automobil nije uspio doći.\n");
            break;

        case 0:
        {
            int position = (rand() % 4);

            printf("Automobil %d došao na poziciju %d.\n", id, position);

            // slanje pozicije ras-u i upr-u
            buf.mtype = MSG_TYPE_CAME;
            buf.mtext[0] = CAR;
            buf.mtext[1] = position;
            if ((msgsnd(id_ras, (struct msgbuf *)&buf, MSG_SIZE, 0)) == -1)
            {
                perror("msgsnd");
            }

            if ((msgsnd(id_upr, (struct msgbuf *)&buf, MSG_SIZE, 0)) == -1)
            {
                perror("msgsnd");
            }

            // ceka zeleno
            //printf("Automobil %d čeka zeleno.\n", id);
            if (position == 1 || position == 3)
            {
                if ((msgrcv(id_sem, (struct msgbuf *)&buf, MSG_SIZE, MSG_TYPE_GO_V, 0)) == -1)
                {
                    perror("msgrcv");
                    exit(1);
                }
            }
            else
            {
                if ((msgrcv(id_sem, (struct msgbuf *)&buf, MSG_SIZE, MSG_TYPE_GO_H, 0)) == -1)
                {
                    perror("msgrcv");
                    exit(1);
                }
            }

            // prolazi kroz raskrizje, salje poruku ras-u radi azuriranja prikaza
            printf("%ld: Automobil %d prolazi kroz raksrižje.\n", time(NULL), id);
            buf.mtype = MSG_TYPE_PASSING;
            buf.mtext[0] = CAR;
            buf.mtext[1] = position;
            if ((msgsnd(id_ras, (struct msgbuf *)&buf, MSG_SIZE, 0)) == -1)
            {
                perror("msgsnd");
            }

            // prolazi kroz raskrizje i nestaje iz sustava, obavjestava ras
            sleep(3);
            //printf("Automobil %d prošao kroz raskrižje.\n", id);
            buf.mtype = MSG_TYPE_LEFT;
            buf.mtext[0] = CAR;
            buf.mtext[1] = position;
            if ((msgsnd(id_ras, (struct msgbuf *)&buf, MSG_SIZE, 0)) == -1)
            {
                perror("msgsnd");
            }

            exit(1);
        }
        }

        ++id;
    }

    //wait(NULL);

    return 0;
}

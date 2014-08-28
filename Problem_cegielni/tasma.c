/*
 * tasma.c
 *
 *  Created on: 29 maj 2014
 *      Author: krzysztof
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <unistd.h>

#include "cegielnia.h"

static struct sembuf buf;

int semid;
int shmid;
int * buffor;

void up(int semid, int semnum, int semvalue) {
    buf.sem_num = semnum;
    buf.sem_op = semvalue;
    buf.sem_flg = 0;
    if (semop(semid, &buf, 1) == -1) {
        perror("Blad przy podnoszeniu semafora");
        exit(1);
    }
}

void down(int semid, int semnum, int semvalue) {
    buf.sem_num = semnum;
    buf.sem_op = -semvalue;
    buf.sem_flg = 0;
    if (semop(semid, &buf, 1) == -1) {
        perror("Blad przy opuszczaniu semafora");
        exit(1);
    }
}

void when_i_am_killed(int sig) {
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {

    int i;
    int j = 0;

    semid = semget(56397, 7, 0600);

    shmid = shmget(56397, K * sizeof(int), 0600);
    if (shmid == -1) {
        perror("Blad przy uzyskiwaniu identyfikatora segmentu pamieci wspoldzielonej");
        exit(1);
    }
    buffor = (int*) shmat(shmid, NULL, 0);
    if (buffor == NULL) {
        perror("Blad przy przylaczaniu segmentu pamieci wspoldzielonej");
        exit(1);
    }

    signal(SIGTSTP, when_i_am_killed);

    while (1) {
        usleep(CONVEYOR_BELT_SPEED);

        down(semid, 4, 1);

        if (buffor[K - 1] != 0)
            down(semid, 0, buffor[K - 1]);
        if (buffor[K - 1] != 0)
            up(semid, 1, buffor[K - 1]);

        j = j + buffor[0];
        if (j == C) {
            up(semid, 6, C);
            j = 0;
        }
        if (buffor[K - 1] != 0) {
            printf("Cegla o wadze [%d] spada z tasmy do ciezarowki.\n",
                    buffor[K - 1]);
            fflush(stdout);
        }
        
        up(semid, 3, buffor[0]);
        up(semid, 2, 1);

        for (i = K - 1; i > 0; i--) {
            buffor[i] = buffor[i - 1];
        }
        buffor[0] = 0;

        up(semid, 5, 1);
    }
    return 0;
}


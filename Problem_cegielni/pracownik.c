/*
 * pracownik.c
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
    int workerId = atoi(argv[1]);

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

    srand(time(NULL) * workerId);

    signal(SIGTSTP, when_i_am_killed);

    printf("Pracownik o identyfikatorze [%d] przyszedl do pracy.\n", workerId);
    fflush(stdout);
    sleep(1);

    while (1) {
        down(semid, 6, workerId);
        down(semid, 2, 1);
        down(semid, 3, workerId);
        down(semid, 5, 1);

        buffor[0] = workerId;
        printf("Pracownik [%d] polozyl na tasme cegle o wadze [%d].\n", workerId, workerId);
        fflush(stdout);

        up(semid, 4, 1);
    }

    return 0;
}

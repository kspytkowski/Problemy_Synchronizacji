/*
 * ciezarowka.c
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
		down(semid, 1, C);
		printf("Zaladowana do pelna ciezarowka odjezdza. Nastepna ciezarowka podstawia sie na koniec tasmy.\n");
		fflush(stdout);
		up(semid, 0, C);
	}

	return 0;
}

/*
 * odczyt_tekstu.c
 *
 *  Created on: 24 maj 2014
 *      Author: krzysztof
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/file.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <pthread.h>

#include "plikKonfiguracyjny.h"

static struct sembuf buf;

void up(int semid, int semnum) {
	buf.sem_num = semnum;
	buf.sem_op = 1;
	buf.sem_flg = 0;
	if (semop(semid, &buf, 1) == -1) {
		perror("Blad przy podnoszeniu semafora");
		exit(1);
	}
}

void down(int semid, int semnum) {
	buf.sem_num = semnum;
	buf.sem_op = -1;
	buf.sem_flg = 0;
	if (semop(semid, &buf, 1) == -1) {
		perror("Blad przy opuszczeniu semafora");
		exit(1);
	}
}

int main() {

	int shmid, semid, i;
	int * buff; // bufor wykorzystywany do komunikacji miedzy nadawcami a odbiorcami
	int s_times[SENDERS] = S_TIMES; // czasy przestoju nadawcow miedzy kolejnymi nadaniami liter
	int r_times[RECEIVERS] = R_TIMES; // czasy przestoju odbiorcow miedzy kolejnymi odbiorami liter

	shmid = shmget(58329, (BUFFER + 2) * sizeof(int), IPC_CREAT | 0600);
	if (shmid == -1) {
		perror("Blad przy tworzeniu segmentu pamieci wspoldzielonej");
		exit(1);
	}
	buff = (int*) shmat(shmid, NULL, 0);
	if (buff == NULL) {
		perror("Blad przy przylaczaniu segmentu pamieci wspoldzielonej");
		exit(1);
	}

#define TO_WRITE_INDEX buff[BUFFER] // index miejsca w buforze, gdzie nalezy wstawic kolejna litere
#define TO_READ_INDEX buff[BUFFER+1] // index miejsca w buforze, skad nalezy przeczytac kolejna litere

	TO_WRITE_INDEX= 0;
	TO_READ_INDEX= 0;

	semid = semget(58329, 4, IPC_CREAT | IPC_EXCL | 0600);
	if (semid == -1) {
		semid = semget(58329, 4, 0600);
		if (semid == -1) {
			perror("Blad przy tworzeniu tablicy semaforow");
			exit(1);
		}
	}
	if (semctl(semid, 0, SETVAL, BUFFER) == -1) { // do kontroli ilosci znakow w buforze
		perror("Blad: Nadanie wartosci semaforowi 0");
		exit(1);
	}
	if (semctl(semid, 1, SETVAL, 0) == -1) { // do kontroli ilosci znakow w buforze
		perror("Blad: Nadanie wartosci semaforowi 1");
		exit(1);
	}
	if (semctl(semid, 2, SETVAL, 1) == -1) { // kontrola dostepu do wspoldzielonego bufora (wsrod nadawcow)
		perror("Blad: Nadanie wartosci semaforowi 2");
		exit(1);
	}
	if (semctl(semid, 3, SETVAL, 1) == -1) { // kontrola dostepu do wspoldzielonego bufora (wsrod odbiorcow)
		perror("Blad: Nadanie wartosci semaforowi 3");
		exit(1);
	}

	int file_to_read = open(FILE_TO_READ, O_RDONLY);

	for (i = 0; i < SENDERS; i++) {
		if (fork() == 0) {
			int nread;
			while (1) {
				down(semid, 0);
				down(semid, 2);
				if ((nread = read(file_to_read, &buff[TO_WRITE_INDEX], sizeof(char))) != 0) {
					TO_WRITE_INDEX = (TO_WRITE_INDEX + 1) % BUFFER;
				}
				else {
					buff[TO_WRITE_INDEX] = -1;
					up(semid, 2);
					up(semid, 1);
					shmdt(buff);
					break;
				}
				up(semid, 2);
				up(semid, 1);
				usleep(s_times[i]);
			}
			return 0;
		}
	}
	for (i = 0; i < RECEIVERS; i++) {
		if (fork() == 0) {
			while (1) {
				down(semid, 1);
				down(semid, 3);
				if (buff[TO_READ_INDEX] != -1) {
					printf("%c", buff[TO_READ_INDEX]);
					fflush(stdout);
					TO_READ_INDEX = (TO_READ_INDEX + 1) % BUFFER;
				}
				else {
					up(semid, 3);
					up(semid, 1);
					shmdt(buff);
					break;
				}
				up(semid, 3);
				up(semid, 0);
				usleep(r_times[i]);
			}
			return 0;
		}
	}

	for (i = 0; i < SENDERS + RECEIVERS; i++)
		wait(0);

	/* Cleaning */
	struct shmid_ds shm_desc;
	shmctl(shmid, IPC_RMID, &shm_desc);
	semctl(semid, 4, IPC_RMID);
	return 0;
}

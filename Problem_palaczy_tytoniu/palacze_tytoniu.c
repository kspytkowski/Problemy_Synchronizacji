/*
 * palacze_tytoniu.c
 *
 *  Created on: 15 maj 2014
 *      Author: krzysztof
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <signal.h>

struct sembuf buf;
int resources_sem; // identyfikator tablicy 3 semaforow broniacych dostepu do 3 grup zasobow opisanych ponizej
char * resources_names[3] = { "papier & tyton", "tyton & zapalki", "zapalki & papier" };
int give_back_to_servant_sem; // identyfikator semaforu synchronizujacego kolejnosc dzialania palaczy i lokaja
int * thread0;
int * thread1;
int * thread2;

void catchCtrlZ(int sig) {
	/* Cleaning up */
	semctl(resources_sem, 3, IPC_RMID);
	semctl(give_back_to_servant_sem, 1, IPC_RMID);

	/* Memory deallocation */
	free(thread0);
	free(thread1);
	free(thread2);

	exit(EXIT_SUCCESS);
}

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
		perror("Blad przy opuszczaniu semafora");
		exit(1);
	}
}

void * servant(void * arg) {
	int random;
	sleep(2);
	while (1) {
		down(give_back_to_servant_sem, 0);
		printf("Lokaj bierze stare zasoby ze stolu\n");
		sleep(1);
		random = rand() % 3;
		printf("Lokaj kladzie na stol %s.\n", resources_names[random]);
		up(resources_sem, random);
	}
	return NULL;
}

void * smoker(void * arg) {
	int palacz_numer_sem = *(int*) arg;
	printf("Palaczowi numer %d brakuje: %s\n", palacz_numer_sem + 1,
			resources_names[palacz_numer_sem]);
	while (1) {
		down(resources_sem, palacz_numer_sem);
		printf("Palacz numer %d bierze %s ze stolu, robi papierosa i pali.\n",
				palacz_numer_sem + 1, resources_names[palacz_numer_sem]);
		sleep(2 + rand() % 4);
		printf("Palacz numer %d oddaje %s na stol.\n", palacz_numer_sem + 1,
				resources_names[palacz_numer_sem]);
		up(give_back_to_servant_sem, 0);
	}
	return NULL;
}

int main(void) {

	pthread_t lokaj_thread, palacz_thread[3];
	int i = 0;

	srand(time(NULL));

	resources_sem = semget(45281, 3, IPC_CREAT | 0600);
	if (resources_sem == -1) {
		resources_sem = semget(45281, 3, 0600);
		if (resources_sem == -1) {
			perror("Blad przy tworzeniu tablicy semaforow");
			exit(1);
		}
	}
	if (semctl(resources_sem, 0, SETVAL, 0) == -1) {
		perror("Blad przy nadawaniu wartosci semaforowi 0");
		exit(1);
	}
	if (semctl(resources_sem, 1, SETVAL, 0) == -1) {
		perror("Blad przy nadawaniu wartosci semaforowi 1");
		exit(1);
	}
	if (semctl(resources_sem, 2, SETVAL, 0) == -1) {
		perror("Blad przy nadawaniu wartosci semaforowi 2");
		exit(1);
	}

	give_back_to_servant_sem = semget(45282, 1, IPC_CREAT | 0600);
	if (give_back_to_servant_sem == -1) {
		give_back_to_servant_sem = semget(45282, 1, 0600);
		if (give_back_to_servant_sem == -1) {
			perror("Blad przy tworzeniu tablicy semaforow");
			exit(1);
		}
	}
	if (semctl(give_back_to_servant_sem, 0, SETVAL, 1) == -1) {
		perror("Blad przy nadawaniu wartosci semaforowi 0");
		exit(1);
	}

	if (pthread_create(&lokaj_thread, NULL, servant, NULL)) {
		printf("Blad przy tworzeniu watku lokaja");
		exit(-1);
	}

	signal(SIGTSTP, catchCtrlZ);

	thread0 = malloc(sizeof(int));
	*thread0 = 0;
	if (pthread_create(&palacz_thread[0], NULL, smoker, (void *) thread0)) {
		printf("Blad przy tworzeniu watku palacza numer %d", 1);
		exit(-1);
	}
	thread1 = malloc(sizeof(int));
	*thread1 = 1;
	if (pthread_create(&palacz_thread[1], NULL, smoker, (void *) thread1)) {
		printf("Blad przy tworzeniu watku palacza numer %d", 2);
		exit(-1);
	}
	thread2 = malloc(sizeof(int));
	*thread2 = 2;
	if (pthread_create(&palacz_thread[2], NULL, smoker, (void *) thread2)) {
		printf("Blad przy tworzeniu watku palacza numer %d", 3);
		exit(-1);
	}

	if (pthread_join(lokaj_thread, NULL)) {
		printf("Blad przy wywolaniu join\n");
		exit(-1);
	}

	for (i = 0; i < 3; i++) {
		if (pthread_join(palacz_thread[i], NULL)) {
			printf("Blad przy wywolaniu join\n");
			exit(-1);
		}
	}

	return 0;
}

/*
 * czytelnicy_i_pisarze.c
 *
 *  Created on: 19 maj 2014
 *      Author: krzysztof
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <unistd.h>

#define AMOUNT_OF_PEOPLE 20

sem_t wait_in_queue_sem; // kontroluje dostep do pliku, zapobiega zaglodzeniu ktorejkolwiek z grup
sem_t access_sem; // umozliwia korzystanie z pliku tylko osobom tej samej grupy (paru czytelnikom albo pisarzowi)
sem_t readers_sem; // synchronizuje dostep do zmiennej actual_amount_of_readers
int actual_amount_of_readers = 0, previous_amount_of_readers = 0,
		current_amount_of_readers = 0;

int writer(void * data) {
	int threadId = *(int*) data;

	int result = sem_wait(&wait_in_queue_sem);
	if (result != 0) {
		fprintf(stderr, "Blad przy zdejmowaniu mutexu.\n");
		exit(-1);
	}
	result = sem_wait(&access_sem);
	if (result != 0) {
		fprintf(stderr, "Blad przy zdejmowaniu mutexu.\n");
		exit(-1);
	}
	result = sem_post(&wait_in_queue_sem);
	if (result != 0) {
		fprintf(stderr, "Blad przy zakladaniu mutexu.\n");
		exit(-1);
	}
	printf("Pisarz o identyfikatorze [%d] zaczal pisac...\n", threadId);
	fflush(stdout);
	sleep(6 + rand() % 12);
	printf("Pisarz o identyfikatorze [%d] skonczyl pisac...\n", threadId);
	fflush(stdout);
	result = sem_post(&access_sem);
	if (result != 0) {
		fprintf(stderr, "Blad przy zakladaniu mutexu.\n");
		exit(-1);
	}
	free(data);
	return 0;
}

int reader(void * data) {
	int threadId = *(int*) data;

	int result = sem_wait(&wait_in_queue_sem);
	if (result != 0) {
		fprintf(stderr, "Blad przy zdejmowaniu mutexu.\n");
		exit(-1);
	}
	result = sem_wait(&readers_sem);
	if (result != 0) {
		fprintf(stderr, "Blad przy zdejmowaniu mutexu.\n");
		exit(-1);
	}
	previous_amount_of_readers = actual_amount_of_readers;
	actual_amount_of_readers++;
	result = sem_post(&readers_sem);
	if (result != 0) {
		fprintf(stderr, "Blad przy zakladaniu mutexu\n");
		exit(-1);
	}
	if (previous_amount_of_readers == 0) {
		int result = sem_wait(&access_sem);
		if (result != 0) {
			fprintf(stderr, "Blad przy zdejmowaniu mutexu.\n");
			exit(-1);
		}
	}
	result = sem_post(&wait_in_queue_sem);
	if (result != 0) {
		fprintf(stderr, "Blad przy zakladaniu mutexu\n");
		exit(-1);
	}
	printf("Czytelnik o identyfikatorze [%d] zaczal czytac...\n", threadId);
	fflush(stdout);
	sleep(4 + rand() % 8);
	printf("Czytelnik o identyfikatorze [%d] skonczyl czytac...\n", threadId);
	fflush(stdout);
	result = sem_wait(&readers_sem);
	if (result != 0) {
		fprintf(stderr, "Blad przy zakladaniu mutexu.\n");
		exit(-1);
	}
	actual_amount_of_readers--;
	current_amount_of_readers = actual_amount_of_readers;
	result = sem_post(&readers_sem);
	if (result != 0) {
		fprintf(stderr, "Blad przy zakladaniu mutexu\n");
		exit(-1);
	}
	if (current_amount_of_readers == 0) {
		result = sem_post(&access_sem);
		if (result != 0) {
			fprintf(stderr, "Blad przy zakladaniu mutexu\n");
			exit(-1);
		}
	}
	free(data);
	return 0;
}

int main(int argc, char* argv[]) {

	pthread_t people[AMOUNT_OF_PEOPLE];
	int i, rc;

	srand(time(NULL));

	sem_init(&wait_in_queue_sem, 0, 1);
	sem_init(&access_sem, 0, 1);
	sem_init(&readers_sem, 0, 1);

	for (i = 0; i < AMOUNT_OF_PEOPLE; i++) {
		sleep(1 + rand() % 2);
		int * threadId = malloc(sizeof(int));
		*threadId = i;

		// pradopodobienstwo dolaczenia do kolejki pisarza jest mniejsze niz czytelnika
		if ((rand() % 12) < 4) {
			rc = pthread_create(&people[i], NULL, (void*) writer,
					(void*) threadId);
			if (rc != 0) {
				fprintf(stderr, "Blad przy tworzeniu watku pisarza");
				exit(-1);
			}
		} else {
			rc = pthread_create(&people[i], NULL, (void*) reader,
					(void*) threadId);
			if (rc != 0) {
				fprintf(stderr, "Blad przy tworzeniu watku czytelnika");
				exit(-1);
			}
		}
	}

	for (i = 0; i < AMOUNT_OF_PEOPLE; i++)
		pthread_join(people[i], NULL);

	return 0;
}

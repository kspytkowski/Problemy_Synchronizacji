/*
 * lazienka.c
 *
 *  Created on: 20 maj 2014
 *      Author: krzysztof
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <semaphore.h>

#define AMOUNT_OF_PEOPLE_IN_QUEUE 20 // ilosc osob w tworzonej kolejce (mezczyzn i kobiet)

sem_t access_sem; // kontroluje dostep do lazienki (zapobiega zaglodzeniu ktorejkolwiek z plci)
sem_t one_sex_in_bathroom_sem; // gwarantuje przebywanie osob jednej plci w lazience w danym momencie
sem_t male_sem; // umozliwia dostep do zmiennej male_couter w danym momencie tylko jednemu watkowi
sem_t three_male_sem; // pilnuje limitu przebywania max. 3 mezczyzn w lazience w danym momencie
int male_counter = 0; // licznik mezczyzn, ktorzy sa w lazience lub oczekuja w kolejce na wejscie (ale przed nimi w kolejce nie ma kobiety)
sem_t female_sem; // umozliwia dostep do zmiennej female_couter w danym momencie tylko jednemu watkowi
sem_t three_female_sem; // pilnuje limitu przebywania max. 3 kobiet w lazience w danym momencie
int female_counter = 0; // licznik kobiet, ktore sa w lazience lub oczekuja w kolejce na wejscie (ale przed nimi w kolejce nie ma mezczyzny)

void * male(void * data) {
    int maleId = *(int*) data;

    int result = sem_wait(&access_sem);
    if (result != 0) {
        fprintf(stderr, "Error when locking the mutex.\n");
        exit(-1);
    }
    result = sem_wait(&male_sem);
    if (result != 0) {
        fprintf(stderr, "Error when locking the mutex.\n");
        exit(-1);
    }
    male_counter++;
    if (male_counter == 1) {
        result = sem_wait(&one_sex_in_bathroom_sem);
        if (result != 0) {
            fprintf(stderr, "Error when locking the mutex.\n");
            exit(-1);
        }
    }
    result = sem_post(&male_sem);
    if (result != 0) {
        fprintf(stderr, "Error when unlocking the mutex.\n");
        exit(-1);
    }
    result = sem_post(&access_sem);
    if (result != 0) {
        fprintf(stderr, "Error when unlocking the mutex.\n");
        exit(-1);
    }
    result = sem_wait(&three_male_sem);
    if (result != 0) {
        fprintf(stderr, "Error when locking the mutex.\n");
        exit(-1);
    }

    printf("Mezczyzna o identyfikatorze [%d] wszedl do lazienki\n", maleId);
    fflush(stdout);
    sleep(4 + rand() % 8);
    printf("Mezczyzna o identyfikatorze [%d] wyszedl z lazienki\n", maleId);
    fflush(stdout);

    result = sem_post(&three_male_sem);
    if (result != 0) {
        fprintf(stderr, "Error when unlocking the mutex.\n");
        exit(-1);
    };
    result = sem_wait(&male_sem);
    if (result != 0) {
        fprintf(stderr, "Error when locking the mutex.\n");
        exit(-1);
    }
    male_counter--;
    if (male_counter == 0) {
        result = sem_post(&one_sex_in_bathroom_sem);
        if (result != 0) {
            fprintf(stderr, "Error when unlocking the mutex.\n");
            exit(-1);
        }
    }
    result = sem_post(&male_sem);
    if (result != 0) {
        fprintf(stderr, "Error when unlocking the mutex.\n");
        exit(-1);
    };
    free(data);
    return NULL;
}

void * female(void * data) {
    int femaleId = *(int*) data;

    int result = sem_wait(&access_sem);
    if (result != 0) {
        fprintf(stderr, "Error when locking the mutex.\n");
        exit(-1);
    }
    result = sem_wait(&female_sem);
    if (result != 0) {
        fprintf(stderr, "Error when locking the mutex.\n");
        exit(-1);
    }
    female_counter++;
    if (female_counter == 1) {
        result = sem_wait(&one_sex_in_bathroom_sem);
        if (result != 0) {
            fprintf(stderr, "Error  when locking the mutex.\n");
            exit(-1);
        }
    }
    result = sem_post(&female_sem);
    if (result != 0) {
        fprintf(stderr, "Error when unlocking the mutex.\n");
        exit(-1);
    }
    result = sem_post(&access_sem);
    if (result != 0) {
        fprintf(stderr, "Error when unlocking the mutex.\n");
        exit(-1);
    }
    result = sem_wait(&three_female_sem);
    if (result != 0) {
        fprintf(stderr, "Error when locking the mutex.\n");
        exit(-1);
    }

    printf("Kobieta o identyfikatorze [%d] weszla do lazienki\n", femaleId);
    fflush(stdout);
    sleep(4 + rand() % 8);
    printf("Kobieta o identyfikatorze [%d] wyszla z lazienki\n", femaleId);
    fflush(stdout);

    result = sem_post(&three_female_sem);
    if (result != 0) {
        fprintf(stderr, "Error when unlocking the mutex.\n");
        exit(-1);
    };
    result = sem_wait(&female_sem);
    if (result != 0) {
        fprintf(stderr, "Error when locking the mutex.\n");
        exit(-1);
    }
    female_counter--;
    if (female_counter == 0) {
        result = sem_post(&one_sex_in_bathroom_sem);
        if (result != 0) {
            fprintf(stderr, "Error when unlocking the mutex.\n");
            exit(-1);
        }
    }
    result = sem_post(&female_sem);
    if (result != 0) {
        fprintf(stderr, "Error when unlocking the mutex.\n");
        exit(-1);
    };
    free(data);
    return NULL;
}

int main(void) {

    pthread_t people[AMOUNT_OF_PEOPLE_IN_QUEUE];
    int i, rc;

    srand(time(NULL));

    sem_init(&access_sem, 0, 1);
    sem_init(&one_sex_in_bathroom_sem, 0, 1);
    sem_init(&male_sem, 0, 1);
    sem_init(&three_male_sem, 0, 3);
    sem_init(&female_sem, 0, 1);
    sem_init(&three_female_sem, 0, 3);

    for (i = 0; i < AMOUNT_OF_PEOPLE_IN_QUEUE; i++) {
        sleep(1);
        int * threadId = malloc(sizeof(int));
        *threadId = i;
        
        // losowo z prawdopodobienstwem 50% tworzymy watki kobiet i mezczyzn
        if ((rand() % 2) == 0) {
            rc = pthread_create(&people[i], NULL, male, (void*) threadId);
            if (rc != 0) {
                fprintf(stderr, "Error when creating male thread");
                exit(-1);
            }
        } else {
            rc = pthread_create(&people[i], NULL, female, (void*) threadId);
            if (rc != 0) {
                fprintf(stderr, "Error when creating female thread");
                exit(-1);
            }
        }
    }

    for (i = 0; i < AMOUNT_OF_PEOPLE_IN_QUEUE; i++)
        pthread_join(people[i], NULL);

    return 0;
}

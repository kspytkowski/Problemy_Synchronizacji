/*
 * cegielnia.c
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

int semid;
int shmid;
int * buffor; // tasma - tablica zawierajaca wartosci mas polozonych na nia cegiel
pid_t child_pids[5];
static struct sembuf buf;

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

void kill_children(int sig) {
    int j;
    for (j = 0; j < 5; j++) {
        kill(child_pids[j], SIGKILL);
    }

    semctl(semid, 7, IPC_RMID);

    struct shmid_ds shm_desc;
    shmctl(shmid, IPC_RMID, &shm_desc);

    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {

    int i;

    semid = semget(56397, 7, IPC_CREAT | IPC_EXCL | 0600);
    if (semid == -1) {
        semid = semget(56397, 7, 0600);
        if (semid == -1) {
            perror("Blad przy tworzeniu tablicy semaforow");
            exit(1);
        }
    }
    if (semctl(semid, 0, SETVAL, C) == -1) { // do do kontroli jenostek masy cegiel spadajacych z konca tasmy do ciezarowki
                                             // (synchronizacja konca tasmy z ciezarowka)
        perror("Blad przy nadawaniu wartosci semaforowi 0");
        exit(1);
    }
    if (semctl(semid, 1, SETVAL, 0) == -1) { // do kontroli jenostek masy cegiel spadajacych z konca tasmy do ciezarowki
                                             // (synchronizacja konca tasmy z ciezarowka)
        perror("Blad przy nadawaniu wartosci semaforowi 1");
        exit(1);
    }
    if (semctl(semid, 2, SETVAL, K) == -1) { // do kontroli ilosci cegiel na tasmie w danej chwili
                                             //(ponizej alokuje bufor o rozmiarze K imitujacy przesuwajaca sie z ceglami tasme produkcyjna)
        perror("Blad przy nadawaniu wartosci semaforowi 2");
        exit(1);
    }
    if (semctl(semid, 3, SETVAL, M) == -1) { // do kontroli masy cegiel na tasmie w danej chwili
        perror("Blad przy nadawaniu wartosci semaforowi 3");
        exit(1);
    }
    if (semctl(semid, 4, SETVAL, 0) == -1) { // do kontroli kolejnosci stawiania przez pracownikow cegiel na tasmie a przesuwaniem sie tasmy
        perror("Blad przy nadawaniu wartosci semaforowi 4");
        exit(1);
    }
    if (semctl(semid, 5, SETVAL, 1) == -1) { // do kontroli kolejnosci stawiania przez pracownikow cegiel na tasmie a przesuwaniem sie tasmy
        perror("Blad przy nadawaniu wartosci semaforowi 5");
        exit(1);
    }
    if (semctl(semid, 6, SETVAL, C) == -1) { // do kontroli odpowiedniej ilosci jednostek cegly kladzionych przez pracownikow na poczatku tasmy
        perror("Blad przy nadawaniu wartosci semaforowi 6");
        exit(1);
    }

    shmid = shmget(56397, K * sizeof(int), IPC_CREAT | 0600);
    if (shmid == -1) {
        perror("Blad przy tworzeniu segmentu pamieci wspoldzielonej");
        exit(1);
    }
    buffor = (int*) shmat(shmid, NULL, 0);
    if (buffor == NULL) {
        perror("Blad przy przylaczaniu segmentu pamieci wspoldzielonej");
        exit(1);
    }

    // poczatkowo na tasmie nie ma zadnej cegly
    for (i = 0; i < K + 1; i++) {
        buffor[i] = 0;
    }

    signal(SIGTSTP, kill_children);

    printf("Ciezarowka podstawia sie na koniec tasmy.\n");
    fflush(stdout);
    child_pids[0] = fork();
    if (child_pids[0] == 0) {
        execlp("./Problem_cegielni_ciezarowka", "Problem_cegielni_ciezarowka", NULL, NULL);
        exit(1);
    }
    printf("Tasma produkcyjna zaczyna sie krecic.\n");
    child_pids[1] = fork();
    if (child_pids[1] == 0) {
        execlp("./Problem_cegielni_tasma", "Problem_cegielni_tasma", NULL, NULL);
        exit(1);
    }
    char * pierwszy = "1";
    child_pids[2] = fork();
    if (child_pids[2] == 0) {
        execlp("./Problem_cegielni_pracownik", "Problem_cegielni_pracownik", pierwszy, NULL);
        exit(1);
    }
    char * drugi = "2";
    child_pids[3] = fork();
    if (child_pids[3] == 0) {
        execlp("./Problem_cegielni_pracownik", "Problem_cegielni_pracownik", drugi, NULL);
        exit(1);
    }
    char * trzeci = "3";
    child_pids[4] = fork();
    if (child_pids[4] == 0) {
        execlp("./Problem_cegielni_pracownik", "Problem_cegielni_pracownik", trzeci, NULL);
        exit(1);
    }

    // czekaj na stworzone powyzej procesy
    for (i = 0; i < 5; i++)
        wait(0);

    return 0;
}

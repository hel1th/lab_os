// child1.c
#include <ctype.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_SIZE 4096
#define DATA_SIZE (SHM_SIZE - sizeof(uint32_t))

const char SHM1_NAME[] = "/shm1";
const char SHM2_NAME[] = "/shm2";
const char SEM1_DATA[] = "/sem1_data";
const char SEM1_FREE[] = "/sem1_free";
const char SEM2_DATA[] = "/sem2_data";
const char SEM2_FREE[] = "/sem2_free";

typedef struct {
    uint32_t length;
    char data[DATA_SIZE];
} shm_buffer_t;

int main() {
    int shm1_fd = shm_open(SHM1_NAME, O_RDWR, 0600);
    if (shm1_fd == -1) {
        perror("child1: shm_open shm1");
        return 1;
    }

    shm_buffer_t* shm1 = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm1_fd, 0);
    if (shm1 == MAP_FAILED) {
        perror("child1: mmap shm1");
        return 1;
    }

    sem_t* sem1_data = sem_open(SEM1_DATA, 0);
    sem_t* sem1_free = sem_open(SEM1_FREE, 0);
    if (sem1_data == SEM_FAILED || sem1_free == SEM_FAILED) {
        perror("child1: sem_open sem1");
        return 1;
    }

    int shm2_fd = shm_open(SHM2_NAME, O_RDWR, 0600);
    if (shm2_fd == -1) {
        perror("child1: shm_open shm2");
        return 1;
    }

    shm_buffer_t* shm2 = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm2_fd, 0);
    if (shm2 == MAP_FAILED) {
        perror("child1: mmap shm2");
        return 1;
    }

    sem_t* sem2_data = sem_open(SEM2_DATA, 0);
    sem_t* sem2_free = sem_open(SEM2_FREE, 0);
    if (sem2_data == SEM_FAILED || sem2_free == SEM_FAILED) {
        perror("child1: sem_open sem2");
        return 1;
    }

    bool running = true;
    while (running) {
        sem_wait(sem1_data);

        if (shm1->length == UINT32_MAX) {
            sem_wait(sem2_free);
            shm2->length = UINT32_MAX;
            sem_post(sem2_data);
            sem_post(sem2_free);
            sem_post(sem1_free);
            break;
        }

        for (uint32_t i = 0; i < shm1->length; i++) {
            shm1->data[i] = toupper((unsigned char)shm1->data[i]);
        }

        sem_wait(sem2_free);
        shm2->length = shm1->length;
        memcpy(shm2->data, shm1->data, shm1->length);
        sem_post(sem2_data);

        sem_post(sem1_free);
    }

    sem_close(sem1_data);
    sem_close(sem1_free);
    sem_close(sem2_data);
    sem_close(sem2_free);
    munmap(shm1, SHM_SIZE);
    munmap(shm2, SHM_SIZE);
    close(shm1_fd);
    close(shm2_fd);

    return 0;
}

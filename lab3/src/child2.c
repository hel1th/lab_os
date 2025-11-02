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

const char SHM2_NAME[] = "/shm2";
const char SHM3_NAME[] = "/shm3";
const char SEM2_DATA[] = "/sem2_data";
const char SEM2_FREE[] = "/sem2_free";
const char SEM3_DATA[] = "/sem3_data";
const char SEM3_FREE[] = "/sem3_free";

typedef struct {
    uint32_t length;
    char data[DATA_SIZE];
} shm_buffer_t;

int main(void) {
    int shm2_fd = shm_open(SHM2_NAME, O_RDWR, 0600);
    if (shm2_fd == -1) {
        perror("child2: shm_open shm2");
        return 1;
    }
    shm_buffer_t* shm2 = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm2_fd, 0);
    if (shm2 == MAP_FAILED) {
        perror("child2: mmap shm2");
        return 1;
    }

    sem_t* sem2_data = sem_open(SEM2_DATA, 0);
    sem_t* sem2_free = sem_open(SEM2_FREE, 0);
    if (sem2_data == SEM_FAILED || sem2_free == SEM_FAILED) {
        perror("child2: sem_open sem2");
        return 1;
    }

    int shm3_fd = shm_open(SHM3_NAME, O_RDWR, 0600);
    if (shm3_fd == -1) {
        perror("child2: shm_open shm3");
        return 1;
    }
    shm_buffer_t* shm3 = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm3_fd, 0);
    if (shm3 == MAP_FAILED) {
        perror("child2: mmap shm3");
        return 1;
    }

    sem_t* sem3_data = sem_open(SEM3_DATA, 0);
    sem_t* sem3_free = sem_open(SEM3_FREE, 0);
    if (sem3_data == SEM_FAILED || sem3_free == SEM_FAILED) {
        perror("child2: sem_open sem3");
        return 1;
    }

    bool running = true;
    while (running) {
        sem_wait(sem2_data);

        if (shm2->length == UINT32_MAX) {
            sem_wait(sem3_free);
            shm3->length = UINT32_MAX;
            sem_post(sem3_data);
            sem_post(sem2_free);
            running = false;
            break;
        }

        for (uint32_t i = 0; i < shm2->length; i++) {
            if (isspace((unsigned char)shm2->data[i])) {
                shm2->data[i] = '_';
            }
        }

        sem_wait(sem3_free);
        shm3->length = shm2->length;
        memcpy(shm3->data, shm2->data, shm2->length);
        sem_post(sem3_data);

        sem_post(sem2_free);
    }

    sem_close(sem2_data);
    sem_close(sem2_free);
    sem_close(sem3_data);
    sem_close(sem3_free);
    munmap(shm2, SHM_SIZE);
    munmap(shm3, SHM_SIZE);
    close(shm2_fd);
    close(shm3_fd);

    return 0;
}
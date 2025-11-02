#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#define SHM_SIZE 4096
#define DATA_SIZE (SHM_SIZE - sizeof(uint32_t))

const char SHM1_NAME[] = "/shm1";
const char SHM2_NAME[] = "/shm2";
const char SHM3_NAME[] = "/shm3";

const char SEM1_DATA[] = "/sem1_data";
const char SEM1_FREE[] = "/sem1_free";
const char SEM2_DATA[] = "/sem2_data";
const char SEM2_FREE[] = "/sem2_free";
const char SEM3_DATA[] = "/sem3_data";
const char SEM3_FREE[] = "/sem3_free";

typedef struct {
    uint32_t length;
    char data[DATA_SIZE];
} shm_buffer_t;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <child1> <child2>\n", argv[0]);
        return 1;
    }

    shm_unlink(SHM1_NAME);
    shm_unlink(SHM2_NAME);


    int shm1_fd = shm_open(SHM1_NAME, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (shm1_fd == -1) {
        perror("shm_open shm1");
        return 1;
    }

    if (ftruncate(shm1_fd, SHM_SIZE) == -1) {
        perror("ftruncate shm1");
        return 1;
    }

    shm_buffer_t* shm1 = mmap(NULL, SHM_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, shm1_fd, 0);
    if (shm1 == MAP_FAILED) {
        perror("mmap shm1");
        return 1;
    }
    shm1->length = 0;


    sem_unlink(SEM1_DATA);
    sem_unlink(SEM1_FREE);

    sem_t* sem1_data = sem_open(SEM1_DATA, O_CREAT | O_EXCL, 0600, 0);
    sem_t* sem1_free = sem_open(SEM1_FREE, O_CREAT | O_EXCL, 0600, 1);

    if (sem1_data == SEM_FAILED || sem1_free == SEM_FAILED) {
        perror("sem_open sem1");
        return 1;
    }


    shm_unlink(SHM2_NAME);
    int shm2_fd = shm_open(SHM2_NAME, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (shm2_fd == -1) {
        perror("shm_open shm2");
        return 1;
    }
    if (ftruncate(shm2_fd, SHM_SIZE) == -1) {
        perror("ftruncate shm2");
        return 1;
    }
    shm_buffer_t* shm2 = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm2_fd, 0);
    if (shm2 == MAP_FAILED) {
        perror("mmap shm2");
        return 1;
    }
    shm2->length = 0;

    sem_unlink(SEM2_DATA);
    sem_unlink(SEM2_FREE);
    sem_t* sem2_data = sem_open(SEM2_DATA, O_CREAT | O_EXCL, 0600, 0);
    sem_t* sem2_free = sem_open(SEM2_FREE, O_CREAT | O_EXCL, 0600, 1);
    if (sem2_data == SEM_FAILED || sem2_free == SEM_FAILED) {
        perror("sem_open sem2");
        return 1;
    }


    shm_unlink(SHM3_NAME);
    int shm3_fd = shm_open(SHM3_NAME, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (shm3_fd == -1) {
        perror("shm_open shm3");
        return 1;
    }
    if (ftruncate(shm3_fd, SHM_SIZE) == -1) {
        perror("ftruncate shm3");
        return 1;
    }
    shm_buffer_t* shm3 = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm3_fd, 0);
    if (shm3 == MAP_FAILED) {
        perror("mmap shm3");
        return 1;
    }
    shm3->length = 0;

    sem_unlink(SEM3_DATA);
    sem_unlink(SEM3_FREE);
    sem_t* sem3_data = sem_open(SEM3_DATA, O_CREAT | O_EXCL, 0600, 0);
    sem_t* sem3_free = sem_open(SEM3_FREE, O_CREAT | O_EXCL, 0600, 1);
    if (sem3_data == SEM_FAILED || sem3_free == SEM_FAILED) {
        perror("sem_open sem3");
        return 1;
    }

    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork child1");
        return 1;
    }
    if (pid1 == 0) {
        execl(argv[1], argv[1], NULL);
        perror("execl child1");
        _exit(2);
    }

    pid_t pid2 = fork();
    if (pid1 == -1) {
        perror("fork child2");
        return 1;
    }
    if (pid2 == 0) {
        execl(argv[2], argv[2], NULL);
        perror("execl child2");
        _exit(2);
    }


    char input_buff[DATA_SIZE];
    bool running = true;

    while (running) {
        ssize_t bytes_cnt = read(STDIN_FILENO, input_buff, sizeof(input_buff));

        if ((bytes_cnt <= 0) || (input_buff[0] == '\n')) {
            sem_wait(sem1_free);
            shm1->length = UINT32_MAX;
            sem_post(sem1_data);
            running = false;
            break;
        }

        sem_wait(sem1_free);
        shm1->length = bytes_cnt;
        memcpy(shm1->data, input_buff, bytes_cnt);
        sem_post(sem1_data);


        sem_wait(sem3_data);
        if (shm3->length == UINT32_MAX) {
            running = false;

        } else if (shm3->length > 0) {
            write(STDOUT_FILENO, shm3->data, shm3->length);
            fputc('\n', stdout);
        }

        sem_post(sem3_free);
    }

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    sem_close(sem1_data);
    sem_close(sem1_free);
    sem_close(sem2_data);
    sem_close(sem2_free);
    sem_close(sem3_data);
    sem_close(sem3_free);

    sem_unlink(SEM1_DATA);
    sem_unlink(SEM1_FREE);
    sem_unlink(SEM2_DATA);
    sem_unlink(SEM2_FREE);
    sem_unlink(SEM3_DATA);
    sem_unlink(SEM3_FREE);

    munmap(shm1, SHM_SIZE);
    munmap(shm2, SHM_SIZE);
    munmap(shm3, SHM_SIZE);

    shm_unlink(SHM1_NAME);
    shm_unlink(SHM2_NAME);
    shm_unlink(SHM3_NAME);

    close(shm1_fd);
    close(shm2_fd);
    close(shm3_fd);

    return 0;
}
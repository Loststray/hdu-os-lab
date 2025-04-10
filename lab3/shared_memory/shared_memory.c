#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>

#define SHM_SIZE 1024 // 共享内存大小

int shmid, alive_task = 0;
sem_t SHM_mutex, M_full, M_empty, M_end;

void *sender(void *arg) {
    if (arg != NULL) {
        return NULL;
    }
    while (1) {
        sem_wait(&M_empty);
        sem_wait(&SHM_mutex);
        char msg[SHM_SIZE];
        char *shared_memory = (char *)shmat(shmid, NULL, 0); // 附加共享内存
        if (shared_memory == (char *)-1) {
            perror("shmat");
            exit(EXIT_FAILURE);
        }
        printf("Sender: Enter message (or 'over' to quit): ");
        scanf("%s", msg);
        msg[strcspn(msg, "\n")] = '\0';
        if (strcmp(msg, "over") == 0) {
            alive_task--;
            strcpy(shared_memory, msg);
            printf("Sender send over. Sender quit\n");
            sem_post(&SHM_mutex);
            sem_post(&M_full);
            shmdt(shared_memory);
            break;
        }
        // 写入数据到共享内存
        strcpy(shared_memory, msg);
        printf("Sender Written to shared memory: %s\n", shared_memory);

        // 分离共享内存
        shmdt(shared_memory);
        sem_post(&SHM_mutex);
        sem_post(&M_full);
    }
    sem_wait(&M_end);
    return NULL;
}

void *receiver(void *arg) {
    if (arg != NULL) {
        return NULL;
    }
    while (alive_task) {
        sem_wait(&M_full);
        sem_wait(&SHM_mutex);
        char *shared_memory = (char *)shmat(shmid, NULL, 0); // 附加共享内存
        // 读取共享内存中的数据

        printf("Receiver: Read from shared memory: %s\n", shared_memory);
        if (strcmp(shared_memory, "over") == 0){
            printf("Receiver: quitting\n");
            sem_post(&SHM_mutex);
            sem_post(&M_empty);
            shmdt(shared_memory);
            break;
        }
        // 分离共享内存
        shmdt(shared_memory);
        sem_post(&SHM_mutex);
        sem_post(&M_empty);
    }
    sem_post(&M_end);
    return NULL;
}

int main(void) {
    key_t key;
    pthread_t t1, t2;
    sem_init(&SHM_mutex, 0, 1);
    sem_init(&M_full, 0, 0);
    sem_init(&M_empty, 0, 1);
    sem_init(&M_end, 0, 0);
    // 创建共享内存的唯一键
    key = ftok("/home/fiatiustitia/hdu-os-lab/lab3/shared_memory/mrbeast",
               114514);
    if (key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // 创建共享内存段
    shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // 创建进程
    pthread_create(&t1, NULL, sender, NULL);
    alive_task = 1;
    pthread_create(&t2, NULL, receiver, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // 删除共享内存段
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}

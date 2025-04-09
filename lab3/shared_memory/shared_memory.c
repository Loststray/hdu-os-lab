#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#define SHM_SIZE 1024 // 共享内存大小

int main() {
    key_t key;
    int shmid;
    char *shared_memory;

    // 创建共享内存的唯一键
    key = ftok("progfile", 65);
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

    // 创建子进程
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // 子进程：写入共享内存
        shared_memory = (char *)shmat(shmid, NULL, 0); // 附加共享内存
        if (shared_memory == (char *)-1) {
            perror("shmat");
            exit(EXIT_FAILURE);
        }

        // 写入数据到共享内存
        strcpy(shared_memory, "Hello from child process!");
        printf("Child: Written to shared memory: %s\n", shared_memory);

        // 分离共享内存
        shmdt(shared_memory);
        exit(0);
    } else {
        // 父进程：读取共享内存
        wait(NULL); // 等待子进程完成

        shared_memory = (char *)shmat(shmid, NULL, 0); // 附加共享内存
        if (shared_memory == (char *)-1) {
            perror("shmat");
            exit(EXIT_FAILURE);
        }

        // 读取共享内存中的数据
        printf("Parent: Read from shared memory: %s\n", shared_memory);

        // 分离共享内存
        shmdt(shared_memory);

        // 删除共享内存段
        shmctl(shmid, IPC_RMID, NULL);
    }

    return 0;
}
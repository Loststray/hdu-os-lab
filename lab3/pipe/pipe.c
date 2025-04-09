#include <assert.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUF_MAX_SIZE 8192 // 定义缓冲区的最大大小

int main(int argc, char **argv) {
    int pipefd[2], pid = 0, i = 0;
    int flag = 0;           // 标志变量，用于控制循环
    ssize_t n;              // 记录读写的字节数
    char buf[BUF_MAX_SIZE]; // 写入管道的缓冲区
    char str[BUF_MAX_SIZE]; // 从管道读取的缓冲区
    sem_t *write_mutex;     // 写入互斥信号量
    sem_t *read_mutex1;     // 子进程 2 的读取信号量
    sem_t *read_mutex2;     // 子进程 3 的读取信号量

    // 创建信号量
    write_mutex = sem_open("pipe_test_wm", O_CREAT | O_RDWR, 0666, 0);
    read_mutex1 = sem_open("pipe_test_rm_1", O_CREAT | O_RDWR, 0666, 0);
    read_mutex2 = sem_open("pipe_test_rm_2", O_CREAT | O_RDWR, 0666, 0);

    // 初始化缓冲区
    memset(buf, 0, BUF_MAX_SIZE);
    memset(str, 0, BUF_MAX_SIZE);

    // 创建管道
    assert(pipe(pipefd) >= 0);

    // 创建第一个子进程
    assert((pid = fork()) >= 0);
    if (pid == 0) {
        // 子进程 1：尝试不断向管道写入数据，直到写入失败
        int count = 0;
        close(pipefd[0]);                      // 关闭管道的读端
        int flags = fcntl(pipefd[1], F_GETFL); // 获取管道写端的文件状态标志
        fcntl(pipefd[1], F_SETFL,
              flags | O_NONBLOCK); // 设置管道写端为非阻塞模式
        while (1) {
            n = write(pipefd[1], buf, BUF_MAX_SIZE); // 写入数据
            if (n == -1)
                break;
            else {
                count++;
                printf("children 1 write %ldB\n", n);
            }
        }
        printf("space = %dKB\n",
               (count * BUF_MAX_SIZE) / 1024); // 打印写入的总数据量
        exit(0);
    }

    // 创建第二个子进程
    assert((pid = fork()) >= 0);
    if (pid == 0) {
        // 子进程 2：写入固定字符串到管道
        sem_wait(write_mutex); // 等待写入信号量
        close(pipefd[0]);      // 关闭管道的读端
        n = write(pipefd[1], "This is the second children.\n",
                  29); // 写入字符串
        printf("children 2 write %ldB\n", n);
        sem_post(write_mutex); // 释放写入信号量
        sem_post(read_mutex1); // 通知主进程读取
        exit(0);
    }

    // 创建第三个子进程
    assert((pid = fork()) >= 0);
    if (pid == 0) {
        // 子进程 3：写入固定字符串到管道
        sem_wait(write_mutex); // 等待写入信号量
        close(pipefd[0]);      // 关闭管道的读端
        n = write(pipefd[1], "This is the third children.\n", 28); // 写入字符串
        printf("children 3 write %ldB\n", n);
        sem_post(write_mutex); // 释放写入信号量
        sem_post(read_mutex2); // 通知主进程读取
        exit(0);
    }

    // 主进程：读取管道中的数据
    wait(0);                               // 等待子进程结束
    close(pipefd[1]);                      // 关闭管道的写端
    int flags = fcntl(pipefd[0], F_GETFL); // 获取管道读端的文件状态标志
    fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK); // 设置管道读端为非阻塞模式
    while (!flag) {
        n = read(pipefd[0], str, BUF_MAX_SIZE); // 从管道读取数据
        if (n == -1) {
            flag = 1; // 读取失败，退出循环
        } else {
            printf("%ldB read\n", n);
        }
    }
    sem_post(write_mutex); // 释放写入信号量

    // 等待子进程 2 和 3 的读取信号量
    sem_wait(read_mutex1);
    sem_wait(read_mutex2);
    n = read(pipefd[0], str, BUF_MAX_SIZE); // 读取剩余数据
    printf("%ldB read\n", n);
    for (i = 0; i < n; i++) {
        printf("%c", str[i]); // 打印读取的数据
    }

    // 关闭并删除信号量
    sem_close(write_mutex);
    sem_close(read_mutex1);
    sem_close(read_mutex2);
    sem_unlink("pipe_test_wm");
    sem_unlink("pipe_test_rm_1");
    sem_unlink("pipe_test_rm_2");
    return 0;
}
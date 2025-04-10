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

#define BUF_MAX_SIZE 8192

int main(void) {
    int pipefd[2];          // 管道文件描述符数组
    int pid;                // 进程ID
    int i = 0;              // 循环变量
    int flag = 0;           // 循环控制标志
    ssize_t n;              // 读写字节数
    char buf[BUF_MAX_SIZE]; // 写入缓冲
    char str[BUF_MAX_SIZE]; // 读取缓冲
    // 初始化缓冲区为0
    memset(buf, 0, BUF_MAX_SIZE);
    memset(str, 0, BUF_MAX_SIZE);

    // 创建/打开三个命名信号量用于进程同步
    sem_t *write_mutex = sem_open("pipe_test_wm", O_CREAT | O_RDWR, 0666, 0);
    sem_t *read_mutex1 = sem_open("pipe_test_rm_1", O_CREAT | O_RDWR, 0666, 0);
    sem_t *read_mutex2 = sem_open("pipe_test_rm_2", O_CREAT | O_RDWR, 0666, 0);

    // 创建管道并检查操作是否成功，0为读端，1为写端
    assert(pipe(pipefd) >= 0);

    // 创建第一个子进程，利用非阻塞写测试管道大小
    assert((pid = fork()) >= 0);
    if (pid == 0) {
        int count = 0;    // 记录写入次数
        close(pipefd[0]); // 关闭读端，不断写入来测试

        // 管道默认是阻塞写，通过`fcntl`设置成非阻塞写，在管道满无法继续写入时返回-EAGAIN，作为循环终止条件
        // 阻塞写时，管道满了之后进程被阻塞，无法设置终止条件从而结束写,所以改成非阻塞写
        int flags = fcntl(pipefd[1], F_GETFL);
        fcntl(pipefd[1], F_SETFL, flags | O_NONBLOCK);
        while (!flag) {
            n = write(pipefd[1], buf, BUF_MAX_SIZE);
            if (n == -1) {
                flag = 1;
            } else {
                count++;
                printf("children 1 write %ldB\n", n);
            }
        }
        // 管道大小
        printf("space = %dKB\n", (count * BUF_MAX_SIZE) / 1024);
        exit(0);
    }

    // 创建第二个子进程：写入特定消息
    assert((pid = fork()) >= 0);
    if (pid == 0) {
        sem_wait(write_mutex); // 等待写互斥信号量
        close(pipefd[0]);
        // 写入消息到管道
        n = write(pipefd[1], "This is the second children.\n", 29);
        printf("children 2 write %ldB\n", n);
        sem_post(write_mutex); // 释放写互斥信号量
        sem_post(read_mutex1); // 通知主进程读取完成
        exit(0);
    }

    // 创建第三个子进程：写入另一条消息，内容同理2
    assert((pid = fork()) >= 0);
    if (pid == 0) {
        sem_wait(write_mutex);
        close(pipefd[0]);
        n = write(pipefd[1], "This is the third children.\n", 28);
        printf("children 3 write %ldB\n", n);
        sem_post(write_mutex);
        sem_post(read_mutex2);
        exit(0);
    }

    wait(0); // 等待第一个子进程结束：父进程必须接收到子进程结束之后返回的
             // 0，才能继续运行，否则阻塞。
    close(pipefd[1]); // 关闭主进程的写端（不影响子进程的写端）

    // 设置非阻塞性读，作为循环结束标志
    int flags = fcntl(pipefd[0], F_GETFL);
    fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);

    // 读取所有可读数据直到管道为空（全为子进程1的数据）
    while (!flag) {
        n = read(pipefd[0], str, BUF_MAX_SIZE);
        if (n == -1) {
            flag = 1;
        } else {
            printf("%ldB read\n", n);
        }
    }

    // mutex初始为0，释放后为正，子进程2，3才能写入
    sem_post(write_mutex);

    // 等待子进程二、三写入完毕，读取的内容子进程2，子进程3的数据
    sem_wait(read_mutex1);
    sem_wait(read_mutex2);
    n = read(pipefd[0], str, BUF_MAX_SIZE);
    printf("%ldB read\n", n);
    for (i = 0; i < n; i++) {
        printf("%c", str[i]);
    }

    sem_close(write_mutex);
    sem_close(read_mutex1);
    sem_close(read_mutex2);
    sem_unlink("pipe_test_wm");
    sem_unlink("pipe_test_rm_1");
    sem_unlink("pipe_test_rm_2");
    return 0;
}

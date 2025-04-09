#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#define MSG_SIZE 128 // 消息内容的最大长度

// 定义消息结构
struct message {
    long msg_type;           // 消息类型
    char msg_text[MSG_SIZE]; // 消息内容
};

int main(void) {
    key_t key;
    int msgid;
    pid_t pid1, pid2, pid3;

    // 创建消息队列的唯一键
    key = ftok("progfile", 65);
    if (key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // 创建消息队列
    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    // 创建第一个子进程
    pid1 = fork();
    if (pid1 == 0) {
        // 子进程 1：发送消息到队列
        struct message msg;
        msg.msg_type = 1; // 消息类型为 1
        strcpy(msg.msg_text, "Hello from child 1");
        msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
        printf("Child 1 sent: %s\n", msg.msg_text);
        exit(0);
    }

    // 创建第二个子进程
    pid2 = fork();
    if (pid2 == 0) {
        // 子进程 2：发送消息到队列
        struct message msg;
        msg.msg_type = 2; // 消息类型为 2
        strcpy(msg.msg_text, "Hello from child 2");
        msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
        printf("Child 2 sent: %s\n", msg.msg_text);
        exit(0);
    }

    // 创建第三个子进程
    pid3 = fork();
    if (pid3 == 0) {
        // 子进程 3：发送消息到队列
        struct message msg;
        msg.msg_type = 3; // 消息类型为 3
        strcpy(msg.msg_text, "Hello from child 3");
        msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
        printf("Child 3 sent: %s\n", msg.msg_text);
        exit(0);
    }

    // 主进程：接收消息
    for (int i = 0; i < 3; i++) {
        struct message msg;
        msgrcv(msgid, &msg, sizeof(msg.msg_text), 0, 0); // 接收任意类型的消息
        printf("Parent received: %s\n", msg.msg_text);
    }

    // 等待子进程结束
    wait(NULL);
    wait(NULL);
    wait(NULL);

    // 删除消息队列
    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}

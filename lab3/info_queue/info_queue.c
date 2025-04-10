#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_MSG_SIZE 256

// 消息结构体
typedef struct msg_st {
    long msg_type; // 消息类型，决定是哪个线程发送的消息
    char msg_text[MAX_MSG_SIZE]; // 消息内容
} msg_st;

int msg_id;
sem_t mq_is_empty, mq_has_msg, sender1_end, sender2_end;

// 发送线程1
void *sender1(void *args) {
    if (args != NULL) {
        return NULL;
    }
    char msg[MAX_MSG_SIZE];
    msg_st message;
    message.msg_type = 1; // sender1 的消息类型为 1

    while (1) {
        sem_wait(&mq_is_empty);
        printf("Sender1: Enter message (or 'exit' to quit): ");
        scanf("%s", msg);
        msg[strcspn(msg, "\n")] = '\0'; // 去掉末尾的换行符

        if (strcmp(msg, "exit") == 0) {
            // 发送结束消息并退出
            strcpy(message.msg_text, "end1");
            msgsnd(msg_id, &message, sizeof(message), 0);
            printf("Sender1: Sent 'end1' to receiver\n");
            sem_post(&mq_has_msg);
            break;
        }

        // 将消息内容放入结构体并发送
        strcpy(message.msg_text, msg);
        msgsnd(msg_id, &message, sizeof(message), 0);
        sem_post(&mq_has_msg);
    }
    sem_wait(&sender1_end);
    printf("sender1: [receiver]: sender1 exit\n");
    return NULL;
}

// 发送线程2
void *sender2(void *args) {
    if (args != NULL) {
        return NULL;
    }
    char msg[MAX_MSG_SIZE];
    msg_st message;
    message.msg_type = 2; // sender2 的消息类型为 2

    while (1) {
        sem_wait(&mq_is_empty);
        printf("Sender2: Enter message (or 'exit' to quit): ");
        scanf("%s", msg);
        msg[strcspn(msg, "\n")] = '\0'; // 去掉末尾的换行符

        if (strcmp(msg, "exit") == 0) {
            // 发送结束消息并退出
            strcpy(message.msg_text, "end2");
            msgsnd(msg_id, &message, sizeof(message), 0);
            printf("Sender2: Sent 'end2' to receiver\n");
            sem_post(&mq_has_msg);
            break;
        }

        // 将消息内容放入结构体并发送
        strcpy(message.msg_text, msg);
        msgsnd(msg_id, &message, sizeof(message), 0);
        sem_post(&mq_has_msg);
    }
    sem_wait(&sender2_end);
    printf("sender1: [receiver]: sender2 exit");
    return NULL;
}

// 接收线程
void *receiver(void *args) {
    if (args != NULL) {
        return NULL;
    }
    int alive_task = 2;
    msg_st message;
    while (alive_task) {
        sem_wait(&mq_has_msg);
        ssize_t bytes_read =
            msgrcv(msg_id, (char *)&message, sizeof(message), 0, 0);
        sem_post(&mq_is_empty);
        if (bytes_read >= 0) {
            // 接收到的消息类型
            if (message.msg_type == 1) {
                printf("Receiver: [Sender1]: %s\n", message.msg_text);
                if (strcmp(message.msg_text, "end1") == 0) {
                    printf("Receiver: Received 'end1' from Sender1. Ending.\n");
                    sem_post(&sender1_end);
                    --alive_task;
                }
            } else if (message.msg_type == 2) {
                printf("Receiver: [Sender2]: %s\n", message.msg_text);
                if (strcmp(message.msg_text, "end2") == 0) {
                    printf("Receiver: Received 'end2' from Sender2. Ending.\n");
                    sem_post(&sender2_end);
                    --alive_task;
                }
            }
        } else {
            perror("mq_receive failed");
        }
    }

    return NULL;
}

int main(void) {
    pthread_t t1, t2, t3;

    key_t key = ftok("/home/fiatiustitia/hdu-os-lab/lab3/info_queue/yaju_senpai", 114514);
    msg_id = msgget(key, IPC_CREAT | 0666);

    if (msg_id == -1) {
        printf("msgget error\n");
        exit(1);
    }

    sem_init(&mq_is_empty, 0, 1);
    sem_init(&mq_has_msg, 0, 0);
    sem_init(&sender1_end, 0, 0);
    sem_init(&sender2_end, 0, 0);

    // 创建三个线程
    if (pthread_create(&t1, NULL, sender1, NULL) != 0) {
        perror("pthread_create(sender1) failed");
        return 1;
    }
    if (pthread_create(&t2, NULL, sender2, NULL) != 0) {
        perror("pthread_create(sender2) failed");
        return 1;
    }
    if (pthread_create(&t3, NULL, receiver, NULL) != 0) {
        perror("pthread_create(receiver) failed");
        return 1;
    }
    // 等待线程结束
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    // 关闭并删除消息队列
    msgctl(msg_id, IPC_RMID, 0);

    return 0;
}

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/sched/signal.h>

MODULE_AUTHOR("Fiatiustitia");
MODULE_LICENSE("GPL");

static struct task_struct *my_thread = NULL;

static int print(void) {
    static struct task_struct *p = NULL;
    for_each_process(p) {
        // check kernel thread
        if (p->mm == NULL) {
            pr_info("%16s %5d %5u %5d %5d\n", p->comm, p->pid, p->__state,
                    p->prio, p->parent->pid);
        }
    }
    return 0;
}

static int __init constructor(void) {
    pr_info("MODULE: 展示内核线程 INIT");
    pr_info("程序名 PID 进程状态 进程优先级 父进程的PID");
    print();
    // my_thread = kthread_run(print, NULL, "new kthread");
    return 0;
}

static void __exit deconstructor(void) { pr_info("MODULE: 展示内核线程 EXIT"); }
module_init(constructor);
module_exit(deconstructor);
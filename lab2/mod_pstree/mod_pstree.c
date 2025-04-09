#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/stat.h>
MODULE_AUTHOR("Fiatiustitia");
MODULE_LICENSE("GPL");

static pid_t PID = 0;

module_param(PID, int, S_IRUGO);

static void show_process_info(struct task_struct *p) {
    if (p)
        pr_info("%16s %5d %5u\n", p->comm, p->pid, p->__state);
}

static void dfs(struct task_struct *p) {
    struct list_head *it = NULL;
    if (!p || !p->pid)
        return;
    list_for_each(it, &p->children) {
        show_process_info(list_entry(it, struct task_struct, sibling));
        dfs(list_entry(it, struct task_struct, sibling));
    }
}

static int print(void) {
    struct pid *ptrpid = find_get_pid(PID);
    struct task_struct *task = NULL;
    struct task_struct *it = NULL;
    if (ptrpid == NULL) {
        pr_info("INVALID PID");
        return 0;
    }
    task = pid_task(ptrpid, PIDTYPE_PID);

    struct list_head *lit = NULL;

    pr_info("SIBLING:");
    list_for_each(lit, &(task->sibling)) {
        show_process_info(list_entry(lit, struct task_struct, sibling));
    }
    pr_info("PARENT:\n");
    show_process_info(task->parent);
    pr_info("CHILDREN:\n");
    dfs(task);
    return 0;
}

static int __init constructor(void) {
    pr_info("MODULE: 输入PID输出进程信息 INIT");
    print();
    // my_thread = kthread_run(print, NULL, "new kthread");
    return 0;
}

static void __exit deconstructor(void) {
    pr_info("MODULE: 输入PID输出进程信息 EXIT");
}
module_init(constructor);
module_exit(deconstructor);
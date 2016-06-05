#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/pid.h>
#include <linux/slab.h>
#include <linux/kthread.h>

MODULE_DESCRIPTION("Affiche périodiquement les stats CPU d'un proc dans syslog");
MODULE_AUTHOR("Marciset");
MODULE_LICENSE("GPL");

struct task_monitor {
	struct pid *tmpid;
	struct task_struct *task;
} *monitStat;

static pid_t monit_pid;
static struct task_struct *monitThread;

module_param_named(target, monit_pid, int, S_IRUGO);
MODULE_PARM_DESC(monit_pid, "PID du processus à monitorer");

/**
 * Récupération de la struct pid de monit_pid
 * @param pid Pid de recherche
 * @return structure en cas de succès
 */
static struct task_monitor *monitor_pid(pid_t pid)
{
	pr_info("[MONITOR_PID] start\n");
	struct task_monitor *mp;

	mp = kzalloc(sizeof(struct task_monitor), GFP_KERNEL);
	if (!mp)
		return NULL;

	mp->tmpid = find_get_pid(pid);
	if (!mp->tmpid) {
		kfree(mp);
		return NULL;
	}

	mp->task = get_pid_task(mp->tmpid, PIDTYPE_PID);

	return mp;
}

int monitor_fn(void *unused)
{
	while (!kthread_should_stop()) {
		if (pid_alive(monitStat->task))
			pr_info("pid %d usr %d sys %d\n", monit_pid,
				(int)monitStat->task->utime,
				(int)monitStat->task->stime);
		else
			pr_info("pid %d done\n", monit_pid);

		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(HZ);
	}

	return 0;
}

static int __init taskmonitor_init(void)
{
	pr_info("[INIT] taskmonitor\n");
	monitStat = monitor_pid(monit_pid);

	if (!monitStat) {
		pr_err("[ERROR] pid not found\n");
		return -1;
	}

	monitThread = kthread_run(monitor_fn, NULL, "monitor : ");

	if (monitThread == ERR_PTR(-ENOMEM)) {
		put_pid(monitStat->tmpid);
		put_task_struct(monitStat->task);
		kfree(monitStat);
		return -1;
	}
	return 0;
}

static void __exit taskmonitor_exit(void)
{
	put_pid(monitStat->tmpid);
	put_task_struct(monitStat->task);
	kfree(monitStat);
	if (monitThread)
		kthread_stop(monitThread);
	pr_info("[EXIT] taskmonitor\n");
}

module_init(taskmonitor_init);
module_exit(taskmonitor_exit);

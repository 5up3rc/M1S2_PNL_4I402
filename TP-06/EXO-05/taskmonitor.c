#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/pid.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/string.h>

MODULE_DESCRIPTION("Module affichant périodiquement les stats CPU d'un proc dans syslog");
MODULE_AUTHOR("Marciset");
MODULE_LICENSE("GPL");

struct task_monitor {
	struct pid *tmpid;
	struct task_struct *task;
} *monitStat;

struct task_sample {
	cputime_t utime;
	cputime_t stime;
};

static pid_t monit_pid;
static struct task_struct *monitThread;

static ssize_t taskmonitor_show(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf);
static ssize_t taskmonitor_store(struct kobject *kobj,
				 struct kobj_attribute *attr,
				 const char *buf, size_t count);
				 
struct kobj_attribute attSysfs = __ATTR_RW(taskmonitor);
static void __exit monitorTask_exit(void);

module_param_named(target, monit_pid, int, S_IRUGO);
MODULE_PARM_DESC(monit_pid, "PID du processus à monitorer");

/**
 * Récupération de la struct pid de monit_pid
 * @param pid Pid de recherche
 * @return 0 en cas de succès
 */
static struct task_monitor *monitor_pid(pid_t pid)
{
	pr_info("[MONITOR_PID] start\n");
	struct task_monitor *mp = kzalloc(sizeof(struct task_monitor),
					  GFP_KERNEL);
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

static int monitor_fn(void *unused)
{
	while (!kthread_should_stop()) {
		if (pid_alive(monitStat->task))
			pr_info("pid %d usr %d sys %d\n", (int)monit_pid,
				(int)monitStat->task->utime,
				(int)monitStat->task->stime);
		else
			pr_info("pid %d done\n", (int)monit_pid);

		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(HZ);
	}

	return 0;
}

static bool get_sample(struct task_monitor *tm, struct task_sample *sample)
{
	if (pid_alive(tm->task)) {
		sample->utime = tm->task->utime;
		sample->stime = tm->task->stime;
		return true;
	}
	return false;
}

static ssize_t taskmonitor_show(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf)
{
	struct task_sample s;
	ssize_t count = 0;

	count = sprintf(buf, "pid %d ", monit_pid);
	if (!get_sample(monitStat, &s))
		count += sprintf(buf+count, "done\n");
	else
		count += sprintf(buf+count, "usr %d sys %d\n",
				 (int)s.utime,
				 (int)s.stime);

	return count;
}

static ssize_t taskmonitor_store(struct kobject *kobj,
				 struct kobj_attribute *attr,
				 const char *buf, size_t count)
{
	if (count < 4)
		return 0;

	if (strcmp(buf, "stop") == 0) {
		if (monitThread) {
			pr_info("[STORE] stop\n");
			kthread_stop(monitThread);
			monitThread = NULL;
		}
		return 4;
	} else if (strcmp(buf, "start") == 0) {
		if (!monitThread) {
			pr_info("[STORE] start\n");
			monitThread = kthread_run(monitor_fn, NULL,
						  "monitor : ");
		}
		return 5;
	}

	return 0;
}

static int __init monitorTask_init(void)
{
	pr_info("[INIT] taskmonitor\n");
	sysfs_create_file(kernel_kobj, &attSysfs.attr);
	monitStat = monitor_pid(monit_pid);

	if (!monitStat) {
		pr_err("[ERROR] pid not found\n");
		sysfs_remove_file(kernel_kobj, &attSysfs.attr);
		return -1;
	}

	monitThread = kthread_run(monitor_fn, NULL, "monitor : ");

	if (monitThread == ERR_PTR(-ENOMEM)) {
		monitorTask_exit();
		return -1;
	}
	return 0;
}

static void __exit monitorTask_exit(void)
{
	put_pid(monitStat->tmpid);
	put_task_struct(monitStat->task);
	kfree(monitStat);
	if (monitThread)
		kthread_stop(monitThread);
	sysfs_remove_file(kernel_kobj, &attSysfs.attr);
	pr_info("[EXIT] taskmonitor\n");
}

module_init(monitorTask_init);
module_exit(monitorTask_exit);

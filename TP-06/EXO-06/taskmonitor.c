#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/pid.h>
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

MODULE_DESCRIPTION("Module affichant périodiquement les stats CPU d'un proc dans syslog");
MODULE_AUTHOR("Marciset");
MODULE_LICENSE("GPL");

#define MAGIC 'N'
#define GET_SAMPLE _IOR(MAGIC, 0, char *)
#define TASKMON_STOP _IOR(MAGIC, 1, void *)
#define TASKMON_START _IOR(MAGIC, 2, void *)
#define TASKMON_SET_PID _IOW(MAGIC, 3, int)

int target;
module_param(target, int, S_IRUSR | S_IWUSR);
MODULE_PARM_DESC(target, "PID du processus à monitorer");


struct task_monitor {
	struct pid *p;
};

struct task_sample {
	cputime_t utime;
	cputime_t stime;
};

static struct task_monitor monitStat;
static struct task_struct *monitThread;
struct task_sample sample;

static int Major;
static const char *name = "taskmonitor";

/**
 * Récupération de la struct pid de monit_pid
 * @param pid Pid de recherche
 * @return 0 en cas de succès
 */
static struct pid *monitor_pid(pid_t pid)
{
	pr_info("[MONITOR_PID] start\n");

	struct pid *p = find_get_pid(pid);

	if (!p) {
		pr_warn("[MONITOR_PID] proc unfound\n");
		return NULL;
	}

	return p;
}

static bool get_sample(struct task_monitor *tm, struct task_sample *s)
{
	struct task_struct *t = get_pid_task(tm->p, PIDTYPE_PID);

	s->utime = t->utime;
	s->stime = t->stime;

	return pid_alive(t);
}

static int monitor_fn(void *unused)
{
	while (!kthread_should_stop()) {
		if (get_sample(&monitStat, &sample))
			pr_info("pid %d usr %d sys %d\n", pid_nr(monitStat.p),
				(int)sample.utime, (int)sample.stime);
		else {
			pr_info("pid %d done\n", pid_nr(monitStat.p));
			break;
		}

		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(HZ);
	}

	return 0;
}

long device_cmd(struct file *filp, unsigned int cmd, unsigned long arg)
{
	char buf[1024];

	sprintf(buf, "pid %d usr %d sys %d\n", pid_nr(monitStat.p),
		(int)sample.utime, (int)sample.stime);

	switch (cmd) {
	case GET_SAMPLE:
		pr_info("[GET_SAMPLE]\n");
		copy_to_user((char *)arg, buf, strlen(buf));
		break;

	case TASKMON_STOP:
		if (monitThread != NULL) {
			pr_info("[TASKMON_STOP]\n");
			kthread_stop(monitThread);
			monitThread = NULL;
		}
		break;

	case TASKMON_START:
		if (monitThread == NULL) {
			pr_info("[TASKMON_START]\n");
			monitThread = kthread_run(monitor_fn, NULL,
						  "monitor_fn");
		}
		break;

	case TASKMON_SET_PID:
		pr_info("[TASKMON_SET_PID] %d\n", (int)arg);
		if (monitStat.p != NULL)
			put_pid(monitStat.p);

		monitStat.p = monitor_pid((pid_t)arg);

		if ((monitStat.p == NULL) &&
		    (monitThread != NULL)) {
			pr_info("[THREAD STOP]\n");
			kthread_stop(monitThread);
			monitThread = NULL;
		} else if ((monitStat.p != NULL) &&
			   (monitThread == NULL)) {
			pr_info("[THREAD START]\n");
			monitThread = kthread_run(monitor_fn, NULL,
						  "monitor_fn");
		}
		break;

	default:
		return -ENOTTY;
	}

	return 0;
}

const struct file_operations fops = {
	.unlocked_ioctl = device_cmd
};

static int __init monitorTask_init(void)
{
	pr_info("[INIT] taskmonitor\n");
	monitStat.p = monitor_pid(target);
	Major = register_chrdev(0, name, &fops);

	pr_info("%s The major device number is %d.\n",
		"Registeration is a success", Major);
	pr_info("mknod /dev/%s c %d 0\n", name, Major);

	if (monitStat.p != NULL)
		monitThread = kthread_run(monitor_fn, NULL, "monitor : ");

	return 0;
}

static void __exit monitorTask_exit(void)
{
	unregister_chrdev(Major, name);
	if (monitThread)
		kthread_stop(monitThread);
	put_pid(monitStat.p);
	pr_info("[EXIT] taskmonitor\n");
}

module_init(monitorTask_init);
module_exit(monitorTask_exit);

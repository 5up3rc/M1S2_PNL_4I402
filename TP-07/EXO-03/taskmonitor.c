#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/pid.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/shrinker.h>
#include <linux/slab.h>

MODULE_DESCRIPTION("A process monitor");
MODULE_AUTHOR("Maxime Lorrillere <maxime.lorrillere@lip6.fr>");
MODULE_LICENSE("GPL");
MODULE_VERSION("1");

#define BUFSIZE 512

unsigned short target = 1; /* default pid to monitor */
unsigned frequency = 1; /* sampling frequency */

module_param(target, ushort, 0400);
module_param(frequency, uint, 0600);


static ssize_t taskmonitor_show(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf);
static ssize_t taskmonitor_store(struct kobject *kobj,
				 struct kobj_attribute *attr,
				 const char *buf, size_t count);
				 
struct kobj_attribute attSysfs = __ATTR_RW(taskmonitor);



struct task_monitor {
	struct pid *pid;
	struct list_head list;
	int nbsamples;
	struct mutex mut;
};

struct task_monitor *tm;

struct task_struct *monitor_thread;

struct task_sample {
	struct list_head list;
	cputime_t utime;
	cputime_t stime;
};

static unsigned long count_func(struct shrinker *, struct_shrink_control *sc);
static unsigned long scan_func(struct shrinker *, struct_shrink_control *sc);

static struct shrinker sh = {
	.count_objects = count_func,
	.scan_objects = scan_func,
	.seeks = DEFAULT_SEEKS,
	.batch = 0,
	.flags = 0
};

struct kmem_cache *task_sample_cachep;





static unsigned long count_func(struct shrinker *, struct_shrink_control *sc)
{
	if (tm)
		return tm->nbsamples;
	else
		return 0;
}

static unsigned long scan_func(struct shrinker *, struct_shrink_control *sc)
{
	unsigned long nbfreed = 0;
	struct task_sample *myTs, *tmpTs;
	
	pr_warn("[SCAN_FUN]\nKernel wants %lu object(s) to free\n", sc->nr_to_scan);
	mutex_lock(&tm->lock);
	
	if (sc->nr_to_scan > 0) {
		/*ARGS : task_sample / task_sample_tmp (because of safe) / 
			task_monitor head list / name of head_list in task_sample struct */
		list_for_each_entry_safe(myTs, tmpTs, &tm->list, list)
		{
			list_del(&myTs->list);
			/*kfree(myTs);*/
			kmem_cache_free(task_sample_cachep, myTs);
			nbfreed++;
			pr_warn("%lu object(s) freed\n", nbfreed);
			if (nbfreed >= sc->nr_to_scan)
				break;
		}
	tm->nbsamples-=nbfreed;
		
	mutex_unlock(&tm->lock);
	return nbfreed;
}

bool get_sample(struct task_monitor *tm, struct task_sample *sample)
{
	struct task_struct *task;
	bool alive = false;

	task = get_pid_task(tm->pid, PIDTYPE_PID);
	if (!task) {
		pr_err("can't find task for pid %u\n", pid_nr(tm->pid));
		goto out;
	}

	task_lock(task);
	alive = pid_alive(task);
	if (alive)
		task_cputime(task, &sample->utime, &sample->stime);
	task_unlock(task);
	put_task_struct(task);
out:
	return alive;
}

void save_sample(void)
{
	struct task_sample *ts =  kmem_cache_alloc(task_sample_cachep, GFP_KERNEL);
				/*	(struct task_sample *)kmalloc(sizeof(struct task_sample));*/
					
	get_sample(tm, ts);
	
	mutex_lock(tm->mut);
	list_add(&(ts->list), &(tm->list));
	mutex_unlock(tm->mut);	
}


void print_sample(struct task_monitor *tm)
{
	struct task_sample ts;
	pid_t pid = pid_nr(tm->pid);
	bool alive;

	alive = get_sample(tm, &ts);

	if (!alive)
		pr_err("%hd is dead\n",	pid);
	else
		pr_info("%hd usr %lu sys %lu\n", pid, ts.utime, ts.stime);
}

int monitor_fn(void *data)
{
	while (!kthread_should_stop()) {
		save_sample();
		set_current_state(TASK_INTERRUPTIBLE);
		if (schedule_timeout(max(HZ/frequency, 1U)))
			return -EINTR;

		print_sample(tm);
	}
	return 0;
}

int monitor_pid(pid_t pid)
{
	struct pid *p = find_get_pid(pid);

	if (!p) {
		pr_err("pid %hu not found\n", pid);
		return -ESRCH;kmonitorfn_thread
	}
	tm = kmalloc(sizeof(*tm), GFP_KERNEL);
	tm->pid = p;
	
	INIT_LIST_HEAD(&tm->list);
	mutex_init(&tm->mut);

	return 0;
}


static ssize_t taskmonitor_show(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf)
{
	struct task_sample s;
	ssize_t count = 0;
	ssize_t count_tmp;
	char buf_tmp[BUFSIZE];

	list_for_each_entry_reverse(s, &tm->list, list)
	{
		count_tmp = 0;
		count_tmp = sprintf(buf_tmp, "pid %d ", monit_pid);
		
		if (!get_sample(tm, &s))
			count_tmp += sprintf(buf_tmp+count_tmp, "done\n");
		else
			count_tmp += sprintf(buf_tmp+count_tmp, "usr %d sys %d\n",
					 (int)s.utime,
					 (int)s.stime);
		
		if(count + count_tmp >= PAGE_SIZE)
			break;
		else {
			strcat(buf, buf_tmp);
			count += count_tmp;
		}
	}
	return count;
}

static ssize_t taskmonitor_store(struct kobject *kobj,
				 struct kobj_attribute *attr,
				 const char *buf, size_t count)
{
	if (count < 4)
		return 0;

	if (strcmp(buf, "stop") == 0) {
		if (monitor_thread) {
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

static int monitor_init(void)
{
	sysfs_create_file(kernel_kobj, &(taskmonitor.attr));
	int err = monitor_pid(target);

	if (err)
		return err;

	task_sample_cachep = kmem_cache_create("task_sample",
							 					sizeof(struct task_sample), 
							 					0,
					       						SLAB_PANIC | SLAB_NOTRACK,
					      						NULL);	
	
	
	register_shrinker(&sh);
	
	monitor_thread = kthread_run(monitor_fn, NULL, "monitor_fn");
	if (IS_ERR(monitor_thread)) {
		err = PTR_ERR(monitor_thread);
		goto abort;
	}

	pr_info("Monitoring module loaded\n");
	return 0;

abort:
	put_pid(tm->pid);
	kfree(tm);
	return err;
}

static void monitor_exit(void)
{
	if (monitor_thread)
		kthread_stop(monitor_thread);
		
	kmem_cache_destroy(task_sample_cachep);

	put_pid(tm->pid);
	kfree(tm);
	
	unregister_shrinker(&sh);
	sysfs_remove_file(kernel_kobj, &(taskmonitor.attr));
	pr_info("Monitoring module unloaded\n");
}

module_init(monitor_init);
module_exit(monitor_exit);


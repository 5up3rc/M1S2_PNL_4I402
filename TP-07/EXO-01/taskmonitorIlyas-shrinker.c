#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/pid.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <asm/page.h>
#include <linux/shrinker.h>

/*******************************************************************************
 * modinfo
 ******************************************************************************/
MODULE_DESCRIPTION("Module \"taskmonitor\" pour noyau linux");
MODULE_AUTHOR("Ilyas Toumlilt, M2 SAR, UPMC");
MODULE_LICENSE("GPL");

/*******************************************************************************
 * args
 ******************************************************************************/
/**
 * @param target PID du processus à monitorer
 */
int target = -1;
module_param(target, int, 0);
MODULE_PARM_DESC(target, "PID du processus a monitorer");

/*******************************************************************************
 * Global
 ******************************************************************************/
/* initialisation de la kattr */
static ssize_t taskmonitor_show(struct kobject *kobj, 
				struct kobj_attribute *attr,
				char *buf);
static ssize_t taskmonitor_store(struct kobject *kobj, 
				 struct kobj_attribute *attr,
				 const char *buf, 
				 size_t count);
struct kobj_attribute taskmonitor_kattr = __ATTR_RW(taskmonitor);

/* struct du processus à monitorer */
struct task_monitor {
        struct pid* target_pid;          /* struct pid du target à monitorer */
        struct task_struct* target_task; /* task_struct du target à monitorer */
	struct list_head list;           /* tête de liste de samples */
	int s_count;                     /* compteur de samples */
	struct mutex lock;               /* verrou d'acces a la liste samples */
};
struct task_monitor* tm = NULL;
static struct task_struct *monitor_thread = NULL;

/* buffer de stockage des statistiques */
struct task_sample {
	struct list_head list;
	cputime_t utime;
	cputime_t stime;
};

/* buffer pour les commandes de control du thread */
#define BUFF_SIZE 16
char cmd_buff[BUFF_SIZE];

/* control du thread de monitoring */
spinlock_t tm_lock;
static void start_tm_thread(void);
static void stop_tm_thread(void);

/* memory management: shrinker */
#define MEM_NB_SAMPLES 10 /* nombre de mac samples à garder en mémoire 
			     après un shrink scan */
static unsigned long
tm_shrink_count(struct shrinker *shrink, struct shrink_control *sc);
static unsigned long
tm_shrink_scan(struct shrinker *shrink, struct shrink_control *sc);
static struct shrinker tm_shrinker = {
	.count_objects = tm_shrink_count,
	.scan_objects  = tm_shrink_scan,
	.seeks         = DEFAULT_SEEKS
};

/*******************************************************************************
 * Implementation
 ******************************************************************************/
/**
 * Récupère la struct pid correspondant au PID passé en argument
 * @param pid le PID a chercher
 * @return 1 si le PID existe, 0 sinon.
 */
int monitor_pid(pid_t pid)
{
	struct pid* tmp = find_get_pid(pid);

	if(!tmp)
		return 0;

	tm = (struct task_monitor*)kmalloc(sizeof(struct task_monitor),
					   GFP_KERNEL);
	tm->target_pid=tmp;
        
	return 1;
}

/**
 * Sauvegarde les statistiques CPU dans la liste des samples
 * Appellée par le kthread.
 */
void save_sample(void)
{
	struct task_struct* ts = get_pid_task(tm->target_pid, PIDTYPE_PID);
	struct task_sample* ret;

	if(unlikely(!ts))
		return;

	ret = (struct task_sample*)
		kmalloc(sizeof(struct task_sample), GFP_KERNEL);

	ret->utime = ts->utime;
	ret->stime = ts->stime;
	
	mutex_lock(&(tm->lock));
	list_add(&(ret->list), &(tm->list));
	tm->s_count++;
	mutex_unlock(&(tm->lock));

	if(ts)
		put_task_struct(ts);
}

/**
 * désalloue un sample
 * WARNING: this func doesn't care about de lock !
 */
void free_sample(struct task_sample *s)
{
	list_del(&(s->list));
	kfree(s);
	tm->s_count--;
}

/**
 * Fonction executee par le thread de monitoring
 */
int monitor_fn(void* unused)
{
        tm->target_task = get_pid_task(tm->target_pid, PIDTYPE_PID);
        
        while(tm->target_task && pid_alive(tm->target_task)
              && !kthread_should_stop()){
		/* sample add */
		save_sample();
		/* 1 sec sleep */
                set_current_state(TASK_INTERRUPTIBLE);
                schedule_timeout(HZ);
        }

        if(tm->target_task)
                put_task_struct(tm->target_task);
        
	pr_warn("monitor_fn: target task is no longer alive !\n");
        
        return 0;
}

static int taskmonitor_init(void)
{
	/* target doit être un entier positif */
        if(unlikely(target <= 0)){
                pr_warn("Usage: taskmonitor.ko target=<PID>\n");
                return -EINVAL;
        }

        /* le target PID doit exister */
        if(unlikely(!monitor_pid(target))){
                pr_warn("taskmonitor error: PID:%d doesn't exist\n", target);
                return -EINVAL;
        }

	/* /sys/kernel/taskmonitor */
	if(unlikely(sysfs_create_file(kernel_kobj, &(taskmonitor_kattr.attr)))){
		pr_warn("taskmonitor: cannot create file\n");
		return -EPERM;
	}

	/* initialisation de la liste de samples */
	INIT_LIST_HEAD(&(tm->list));
	tm->s_count = 0;
	mutex_init(&(tm->lock));
	
	/* j'initialise le spinlock */
	spin_lock_init(&tm_lock);

	/* je demarre le thread de monitoring */
	start_tm_thread();

	/* je demarre le shrinker */
	register_shrinker(&tm_shrinker);
	
	pr_info("insmod: inserted module taskmonitor\n");        
	return 0;
}
module_init(taskmonitor_init);

static void taskmonitor_exit(void)
{
	struct task_sample *ptr, *n;

	stop_tm_thread();

	sysfs_remove_file(kernel_kobj, &(taskmonitor_kattr.attr));
	
	unregister_shrinker(&(tm_shrinker));

	list_for_each_entry_safe(ptr, n, &(tm->list), list){
		list_del(&(ptr->list));
		kfree(ptr);
	}

	if(tm){
                put_pid(tm->target_pid);
                kfree(tm);
        }
	
	pr_info("rmmod: removed module taskmonitor\n");
}
module_exit(taskmonitor_exit);

static ssize_t taskmonitor_show(struct kobject *kobj, 
				struct kobj_attribute *attr,
				char *buf)
{
	/* pour ne pas dépasser les PAGE_SIZE octets, taille du buffer
	   du sysfs, je vais laisser ( au pire des cas ) 128 octets
	   libres à la fin du buffer */
	int str_size = 0;
	char tmpbuf[64];
	struct task_sample* ptr;
	buf[0] = '\0';
	mutex_lock(&(tm->lock));
	list_for_each_entry(ptr, &(tm->list), list){
		str_size += sprintf(tmpbuf, "pid %d usr %d sys %d\n", target,
				    (int)ptr->utime, (int)ptr->stime);
		strcat(buf, tmpbuf);
		if(str_size >= PAGE_SIZE-128)
			break;
	}
	mutex_unlock(&(tm->lock));
	
	return (ssize_t)str_size;
}

static ssize_t taskmonitor_store(struct kobject *kobj, 
				 struct kobj_attribute *attr,
				 const char *buf, 
				 size_t count)
{
	if(!strcmp("start", buf)){
		start_tm_thread();
	}
	else if(!strcmp("stop", buf)){
		stop_tm_thread();
	}
	return count;
}

static void start_tm_thread(void)
{
	spin_lock(&tm_lock);
	/* je démarre le thread de monitoring si ce n'est déjà fait */
	if(!monitor_thread)
		monitor_thread = kthread_run(monitor_fn, NULL, "monitor_fn");
	spin_unlock(&tm_lock);
}

static void stop_tm_thread(void)
{
	spin_lock(&tm_lock);
	if(monitor_thread){
		kthread_stop(monitor_thread);
		monitor_thread = NULL;
	}
	spin_unlock(&tm_lock);
}

/*******************************************************************************
 * Memory Management: Shrinker
 ******************************************************************************/
static unsigned long
tm_shrink_count(struct shrinker *shrink, struct shrink_control *sc)
{
	long ret;

	mutex_lock(&(tm->lock));
	ret = tm->s_count - MEM_NB_SAMPLES;
	if(unlikely(ret<0))
		ret = 0;
	mutex_unlock(&(tm->lock));
	return (unsigned long)ret;
}
static unsigned long
tm_shrink_scan(struct shrinker *shrink, struct shrink_control *sc)
{
	long ret = 0;
	struct task_sample *ptr, *n;
	
	mutex_lock(&(tm->lock));
	list_for_each_entry_safe_reverse(ptr, n, &(tm->list), list){
		if(unlikely(tm->s_count-MEM_NB_SAMPLES<=0))
			break;
		free_sample(ptr);
		ret++;
	}
	mutex_unlock(&(tm->lock));
	
	pr_info("taskmonitor: sample data shrinked with a %d objects removal\n",
		(int)ret);

	return ret;
}

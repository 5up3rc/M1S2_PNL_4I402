#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/pid.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/sysinfo.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/kobject.h>

#include "projet.h"

MODULE_DESCRIPTION("Module creant un nouveau driver");
MODULE_AUTHOR("Quentin Marciset");
MODULE_LICENSE("GPL");

DECLARE_WAIT_QUEUE_HEAD(cond_wait_queue);

/* workers structures */
struct list_work_t {
	struct work_struct my_work;
	char * buf;
};

struct killer_work_t {
	struct work_struct my_work;
	struct pidsig * ps;
};

struct wait_work_t {
	struct work_struct my_work;
	struct pidbuf * pb;
};

struct meminfo_work_t {
	struct work_struct my_work;
	struct sysinfo * val;
};

struct modinfo_work_t {
	struct work_struct my_work;
	struct namebuf * nb;
};

/* waitqueue workers */
struct list_work_t *list_work;
struct killer_work_t *killer_work;
struct wait_work_t *wait_work;
struct meminfo_work_t *meminfo_work;
struct modinfo_work_t *modinfo_work;

/* waitqueue waiting conditions */
static int list_cond;
static int killer_cond;
static int wait_cond;
static int meminfo_cond;
static int modinfo_cond;


/*******************************************************************************
 ** functions for workers ******************************************************
 *
 */

/**
 * lists the active jobs
 */
static void list_wq_function (struct work_struct *work)
{
	struct list_work_t *tmp_work;
	struct module *mod;
	int free = 256;
	char tmp[256];
	
	mod = THIS_MODULE;
	
	tmp_work = container_of(work, struct list_work_t, my_work);
	
	free -= scnprintf(tmp, 256, "%s / %u / %d \n", mod->name, 
		mod->core_size, atomic_read(&(mod->mkobj.kobj.kref.refcount))); 
	
	strncat(tmp_work->buf, tmp, free);
	
	list_for_each_entry(mod, &(mod->list), list) {
		if(mod->state == 0) {
			free -= scnprintf(tmp, 256, "%s / %u / %d \n", 
				mod->name, 
				mod->core_size, 
				atomic_read(&(mod->mkobj.kobj.kref.refcount)));
			
			strncat(tmp_work->buf, tmp, free); 
		}
	}
	
	list_cond = 1;
	wake_up(&cond_wait_queue);
	return;
}

/**
 * kills a process
 */
static void killer_wq_function(struct work_struct *work)
{
	struct killer_work_t *tmp_work;
	struct pid *p;
  
	tmp_work = container_of(work, struct killer_work_t, my_work);
    
	p = find_get_pid(tmp_work->ps->pid);
	if (!p) {
		pr_debug("[KILL] PID NOT FOUND \n");
		goto ending;
	}
  
	kill_pid(p, tmp_work->ps->sig, 1);
	put_pid(p);
  
ending:
	killer_cond = 1;
	wake_up(&cond_wait_queue);
	return;
}

/**
 * waits the end of a process
 * gets the dead process return value
 */
static void wait_wq_function(struct work_struct *work)
{	
	struct wait_work_t *tmp_work;
	struct task_struct **ts;
	int i, howmany = 0;
	
	ts = kmalloc(sizeof(struct task_struct *), GFP_KERNEL);
	
	tmp_work = container_of(work, struct wait_work_t, my_work);
	
	/* get the 'howmany' task_struct for 'howmany' pids we have */
	for ( i=0 ; i < 10 ; i++) {
		if(tmp_work->pb->pid[i] == 0){
			howmany = i-1;
			break;
		}
		ts[i] = pid_task(find_vpid(tmp_work->pb->pid[i]), PIDTYPE_PID);
	}
	
	i=0;
	while (1) {
		if (i == howmany)
			i=0;
		/* ts->state  -1 unrunnable, 0 runnable, >0 stopped  */
		if (ts[i]->state > 0) 
			break;
		
		i++;
	}
	
	strcpy(tmp_work->pb->buf, "PID : ");
	snprintf(tmp_work->pb->buf, sizeof(int), "%d", tmp_work->pb->pid[i]);
	strcat(tmp_work->pb->buf, " , Valeur de retour : ");
	snprintf(tmp_work->pb->buf, sizeof(ts[i]->exit_code), "%d", 
							ts[i]->exit_code);
	strcat(tmp_work->pb->buf, "\n");
	
	wait_cond = 1;
	/*wake_up(&cond_wait_queue);*/
	return;
}

/**
 * gives information about memory
 */
static void meminfo_wq_function(struct work_struct *work)
{
	struct meminfo_work_t *tmp_work;
	
	tmp_work = container_of(work, struct meminfo_work_t, my_work);
	si_meminfo(tmp_work->val);
	
	meminfo_cond = 1;
	wake_up(&cond_wait_queue);
	return;
}

/**
 * gives information about a module
 */
static void modinfo_wq_function(struct work_struct *work)
{
	struct modinfo_work_t *tmp_work;
	struct module *mod;
	int free = 512;
	char tmp[512];

	tmp_work = container_of(work, struct modinfo_work_t, my_work);
		
	mod = find_module(tmp_work->nb->name);
			
	free -= scnprintf(tmp, 256, "%s / %s / %s / %p \n", mod->name, 
			mod->version, mod->modinfo_attrs.attr->name, &mod);
	
	strncat(tmp_work->nb->buf, tmp, free);
	
	modinfo_cond = 1;
	wake_up(&cond_wait_queue);
	return;
}
/*
 *
 ** end of functions for workers ***********************************************
 ******************************************************************************/


/**
 * data traffic from/to user space
 */
long device_cmd(struct file *filp, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {

	case LIST: /* schedule LIST work */
		list_cond = 0;
		
		list_work->buf = kmalloc(sizeof(char)*256, GFP_KERNEL);
		if (!list_work->buf) {
			pr_debug("[LIST] kmalloc buf failed\n");
			break;
		}
		
		schedule_work(&list_work->my_work);
		wait_event(cond_wait_queue, list_cond);
		
		if (copy_to_user((char *)arg, list_work->buf, 
							sizeof(char)*256)) {
			pr_debug("[LIST] copy_to_user failed\n");
			return -EFAULT;
		}
		
		kfree(list_work->buf);
		
		break;
		
	case KILL: /* schedule KILL work */
		
		killer_cond = 0;
		
		killer_work->ps = kmalloc(sizeof(struct pidsig), GFP_KERNEL);
		if (!killer_work->ps) {
			pr_debug("[KILL] kmalloc failed\n");
			break;
		}
		
		if (copy_from_user(killer_work->ps, (struct pidsig *)arg, 
						sizeof(struct pidsig))) {
			pr_debug("[KILL] copy_from_user failed\n");
			return -EACCES;			
		}
														
		schedule_work(&killer_work->my_work);
		wait_event(cond_wait_queue, killer_cond);
		
		kfree(killer_work->ps);
		
		break;
		
	case WAIT: /* schedule WAIT work */
		/* consider it being able to receive between 1 and 10 pids */
	
		wait_cond = 0;
		
		wait_work->pb = kmalloc(sizeof(struct pidbuf), GFP_KERNEL);
		
		copy_from_user(wait_work->pb->pid, (int *)arg, 
						sizeof(int)*10);
		
		schedule_work(&wait_work->my_work);
		/*wait_event(cond_wait_queue, wait_cond);*/
		flush_work(&wait_work->my_work);
		
		kfree(wait_work->pb);
	
		break;
		
	case MEMINFO: /* schedule MEMINFO work */
		
		meminfo_cond = 0;
		
		meminfo_work->val = kmalloc(sizeof(struct sysinfo), GFP_KERNEL);
		if (!meminfo_work->val) {
			pr_debug("[MEMINFO] kmalloc failed\n");
			break;
		}
		
		schedule_work(&meminfo_work->my_work);
		wait_event(cond_wait_queue, meminfo_cond);
		
		if(copy_to_user((char *)arg, meminfo_work->val, 
						sizeof(struct sysinfo))) {
			pr_debug("[MEMINFO] copy_to_user failed\n");
			return -EFAULT;
		}
		
		kfree(meminfo_work->val);
		
		break;
		
	case MODINFO: /* schedule MODINFO work */
		
		modinfo_cond = 0;
		
		modinfo_work->nb = kmalloc(sizeof(struct namebuf), GFP_KERNEL);
		if (!modinfo_work->nb) {
			pr_debug("[MODINFO] kmalloc failed\n");
			break;
		}
		
		if (copy_from_user(modinfo_work->nb->name, (char *)arg, 
							sizeof(char)*64)) {
			pr_debug("[MODINFO] copy_from_user failed\n");
			return -EACCES;			
		}
		
		schedule_work(&modinfo_work->my_work);
		wait_event(cond_wait_queue, modinfo_cond);
		
		if (copy_to_user((char *)arg, modinfo_work->nb->buf, 
							sizeof(char)*512)) {
			pr_debug("[MODINFO] copy_to_user failed\n");
			return -EFAULT;
		}
		
		kfree(modinfo_work->nb);
		
		break;

	default:
		return -ENOTTY;
	}

	return 0;
}

const struct file_operations fops = {
	.unlocked_ioctl = device_cmd
};

/**
 * init function, to be called on insmod
 * returns 0 on success
 */
static int __init projet_init(void)
{
	pr_info("Starting...\n");
	major = register_chrdev(0, name, &fops);

	if (major < 0) {
		pr_info("[PROJET] %s failed with %d\n",
			"Sorry, registering the character device ", major);
		return major;
	}
	
	/* initialize list work struct */
	list_work = kmalloc(sizeof(struct list_work_t), GFP_KERNEL);
	if (!list_work) {
		pr_debug("[INIT] kmalloc list_work failed\n");
		goto ending;
	}
	INIT_WORK(&list_work->my_work, list_wq_function);

	/* initialize kill work struct */
	killer_work = kmalloc(sizeof(struct killer_work_t), GFP_KERNEL);
	if (!killer_work) {
		pr_debug("[INIT] kmalloc killer_work failed\n");
		goto ending;
	}
	INIT_WORK(&killer_work->my_work, killer_wq_function);
	
	/* initialize wait work struct */
	wait_work = kmalloc(sizeof(struct wait_work_t), GFP_KERNEL);
	if (!wait_work) {
		pr_debug("[INIT] kmalloc wait_work failed\n");
		goto ending;
	}
	INIT_WORK(&wait_work->my_work, wait_wq_function);
	
	/* initialize meminfo work struct */
	meminfo_work = kmalloc(sizeof(struct meminfo_work_t), GFP_KERNEL);
	if (!meminfo_work) {
		pr_debug("[INIT] kmalloc meminfo_work failed\n");
		goto ending;
	}
	INIT_WORK(&meminfo_work->my_work, meminfo_wq_function);
	
	/* initialize modinfo work struct */
	modinfo_work = kmalloc(sizeof(struct modinfo_work_t), GFP_KERNEL);
	if (!modinfo_work) {
		pr_debug("[INIT] kmalloc modinfo_work failed\n");
		goto ending;
	}
	INIT_WORK(&modinfo_work->my_work, modinfo_wq_function);

ending:
	return 0;
}

/**
 * exit function, to be called on rmmod
 */
static void __exit projet_exit(void)
{
	kfree(list_work);
	kfree(killer_work);	
	kfree(wait_work);
	kfree(meminfo_work);
	kfree(modinfo_work);
	
	unregister_chrdev(major, name);
	pr_info("Ending...\n");
}

module_init(projet_init);
module_exit(projet_exit);

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sysfs.h>

MODULE_DESCRIPTION("kernel module saying hello with sysfs");
MODULE_AUTHOR("Marciset");
MODULE_LICENSE("GPL");

static ssize_t hello_show(struct kobject *kobj, struct kobj_attribute *attr,
			  char *buf);
static ssize_t hello_store(struct kobject *kobj, struct kobj_attribute *attr,
			   const char *buf, size_t count);
                     
static struct kobj_attribute attrhello = __ATTR_RW(hello); /*prefixe des _show _store*/
static char name[PAGE_SIZE] = "hellox"; /*check si le fichier créé à ce nom là,
			ou le nom donné dans la ligne précédente puis ligne 22*/ /*réponse : NON, c'est uniquement ce qui est contenu dans le 'fichier' /sys/kernel/hello */
int file;

static int hellosysfs_init(void)
{
	file = sysfs_create_file(kernel_kobj, &attrhello.attr);
	pr_info("module loaded\n");
	return 0;
}

static void hellosysfs_exit(void)
{
	sysfs_remove_file(kernel_kobj, &attrhello.attr);	
	pr_warn("module unloaded\n");
}

static ssize_t hello_show(struct kobject *kobj, struct kobj_attribute *attr,
			  char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%s\n", name);
}

/*store() should return the number of bytes used from the buffer. If the
  entire buffer has been used, just return the count argument.*/
static ssize_t hello_store(struct kobject *kobj, struct kobj_attribute *attr,
			   const char *buf, size_t count)
{
	if(count > PAGE_SIZE)
		return count;
	snprintf(name, PAGE_SIZE, "%s", buf);
	return PAGE_SIZE;
}


module_init(hellosysfs_init);
module_exit(hellosysfs_exit);

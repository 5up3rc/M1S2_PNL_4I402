#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/uaccess.h>

MODULE_DESCRIPTION("Module creant un nouveau driver");
MODULE_AUTHOR("Marciset");
MODULE_LICENSE("GPL");

#define MAGIC 'N'
#define HELLO _IOR(MAGIC, 0, char *)
#define WHO _IOR(MAGIC, 1, char *)

static int major;
static const char *name = "hello";
static char msg[1024] = "hello ioctl !";

long device_cmd(struct file *filp, unsigned int cmd, unsigned long arg)
{
	char buf[1024];

	switch (cmd) {
	case HELLO:
		copy_to_user((char *)arg, msg, strlen(msg));
		break;

	case WHO:
		copy_from_user(buf, (char *)arg, strlen(buf));
		strcpy(msg, "hello ");
		strcat(msg, buf);
		strcat(msg, " !");
		break;

	default:
		return -ENOTTY;
	}

	return 0;
}

const struct file_operations fops = {
	.unlocked_ioctl = device_cmd
};


static int helloioctl_init(void)
{
	major = register_chrdev(0, name, &fops);

	if (major < 0) {
		pr_info("[HELLOIOCTL] %s failed with %d\n",
			"Sorry, registering the character device ", major);
		return major;
	}

	pr_info("%s The major device number is %d.\n",
		"Registeration is a success", major);
	pr_info("mknod /dev/%s c %d 0\n", "hello", major);

	return 0;
}

static void helloioctl_exit(void)
{
	unregister_chrdev(major, name);
	pr_info("%s The major device number is %d.\n",
		"Unregisteration is a success", major);
}


module_init(helloioctl_init);
module_exit(helloioctl_exit);

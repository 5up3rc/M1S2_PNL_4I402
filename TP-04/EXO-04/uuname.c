#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/moduleparam.h>

MODULE_DESCRIPTION("Module \"uuname\" pour noyau linux");
MODULE_AUTHOR("Julien Sopena, LIP6");
MODULE_LICENSE("GPL");

static void printsb (struct super_block *sb, void *p) {
	pr_info("uuid=%x type=%s\n",sb->s_uuid, sb->s_type.name);
}

static int hello_init(void)
{
	iterate_supers((void)printsb(struct super_block*, void*));
	return 0;
}
module_init(hello_init);

static void hello_exit(void)
{
	pr_info("module decharge\n");
}
module_exit(hello_exit);


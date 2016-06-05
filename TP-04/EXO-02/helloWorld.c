#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>

MODULE_DESCRIPTION("Module \"hello word\" pour noyau linux");
MODULE_AUTHOR("Julien Sopena, LIP6");
MODULE_LICENSE("GPL");

static char *whom = "default";
module_param(whom, charp, 0660);
static int howmany = 1;
module_param(howmany, int, 0660);

static int hello_init(void)
{
	int i;
	for(i=0; i< howmany; i++){
		pr_info("(%d) Hello, %s\n", i, whom);
	}
	return 0;
}
module_init(hello_init);

static void hello_exit(void)
{
	pr_info("Goodbye, %s\n", whom);
}
module_exit(hello_exit);


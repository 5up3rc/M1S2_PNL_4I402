#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/moduleparam.h>

MODULE_DESCRIPTION("Module \"uuname\" pour noyau linux");
MODULE_AUTHOR("Julien Sopena, LIP6");
MODULE_LICENSE("GPL");


static int hello_init(void)
{
	strcpy(init_uts_ns.name.sysname, "monnoyauperso");
	return 0;
}
module_init(hello_init);

static void hello_exit(void)
{
	strcpy(init_uts_ns.name.sysname, "vm-pnl");
}
module_exit(hello_exit);


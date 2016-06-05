#include <linux/module.h>
#include <linux/timer.h>
#include <linux/kernel_stat.h>

MODULE_DESCRIPTION("A pr_debug kernel module");
MODULE_AUTHOR("Maxime Lorrillere <maxime.lorrillere@lip6.fr>");
MODULE_LICENSE("GPL");

static struct timer_list prdebug_timer;

static void prdebug_timeout(unsigned long data)
{
	pr_debug("nr irqs %lu\n", kstat_cpu(0).irqs_sum - data);
	prdebug_timer.data = kstat_cpu(0).irqs_sum;
	mod_timer(&prdebug_timer, jiffies + HZ);
}

static int prdebug_init(void)
{
	init_timer(&prdebug_timer);

	prdebug_timer.data = kstat_cpu(0).irqs_sum;
	prdebug_timer.function = prdebug_timeout;
	prdebug_timer.expires = jiffies + HZ;

	add_timer(&prdebug_timer);

	pr_info("prdebug module loaded\n");
	return 0;
}

static void prdebug_exit(void)
{
	del_timer_sync(&prdebug_timer);
	pr_info("prdebug module unloaded\n");
}

module_init(prdebug_init);
module_exit(prdebug_exit);

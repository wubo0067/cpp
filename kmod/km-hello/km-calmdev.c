#include <linux/kernel.h> /* We're doing kernel work */
#include <linux/module.h> /* Specifically, a module */
#include <linux/proc_fs.h>    /* Necessary because we use the proc fs */
#include <asm/uaccess.h>  /* for copy_from_user */
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>

#define BUFSIZE 128

static int irq = 20;
module_param(irq, int, 0660);

static int mode = 1;
module_param(mode, int, 0660);

static struct proc_dir_entry *ent;

static ssize_t calmdev_write(struct file *file, const char __user *ubuf,
                             size_t count, loff_t *ppos) {
    int num, c, i, m;
    char buf[BUFSIZE];
    if (*ppos > 0 || count > BUFSIZE) return -EFAULT;
    if (copy_from_user(buf, ubuf, count)) return -EFAULT;
    num = sscanf(buf, "%d %d", &i, &m);
    if (num != 2) return -EFAULT;
    irq = i;
    mode = m;
    c = strlen(buf);
    *ppos = c;
    return c;
}

static ssize_t calmdev_read(struct file *file, char __user *ubuf, size_t count,
                            loff_t *ppos) {
    char buf[BUFSIZE];
    int len = 0;
    if (*ppos > 0 || count < BUFSIZE) return 0;
    len += sprintf(buf, "irq = %d\n", irq);
    len += sprintf(buf + len, "mode = %d\n", mode);

    if (copy_to_user(ubuf, buf, len)) return -EFAULT;
    *ppos = len;
    return len;
}

static int calmdev_show(struct seq_file *m, void *v) {
    static char * str = NULL;
    seq_printf(m, "%s\n", str);
    return 0;
}

static int calmdev_open(struct inode *inode, struct file *file) {
    return single_open(file, calmdev_show, NULL);
}

static struct proc_ops calmdev_ops = {
    .proc_lseek = seq_lseek,
    .proc_read = calmdev_read,
    .proc_write = calmdev_write,
    .proc_open = calmdev_open,
    .proc_release = single_release,
};

static int calmdev_init(void) {
    ent = proc_create("calmdev", 0666, NULL, &calmdev_ops);
    printk(KERN_WARNING "Module init: hello calm dev!\n");
    return 0;
}

static void calmdev_exit(void) {
    proc_remove(ent);
    printk(KERN_WARNING "Module exit: bye calm dev\n");
}

module_init(calmdev_init);
module_exit(calmdev_exit);

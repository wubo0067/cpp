#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

static int irq = 20;
module_param(irq, int, 0660);

static int mode = 1;
module_param(mode, int, 0660);

static struct proc_dir_entry *ent;

static ssize_t calmdev_write(struct file *file, const char __user *buf,
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

static ssize_t calmdev_read(struct file *file, char __user *buf, size_t count,
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

static struct file_operations calmdev_ops = {
    .owner = THIS_MODULE,
    .read = calmdev_read,
    .write = calmdev_write,
};

static int calmdev_init(void) {
    ent = proc_create("calmdev", 0666, NULL, &calmdev_ops);
    printk(KERN_WARNING "Module init: hello calm dev!\n");
    return 0;
}

static void hello_exit(void) {
    proc_remove(ent);
    printk(KERN_WARNING "Module exit: bye calm dev\n");
}

module_init(hello_init);
module_exit(hello_exit);

#include <linux/init.h>  
#include <linux/kernel.h>  
#include <linux/module.h>  
  
static int hello_init(void) {  
    printk(KERN_WARNING "Module init: Hello world!\n");  
    return 0;  
}  
  
static void hello_exit(void) {  
    printk(KERN_WARNING "Module exit: bye-bye\n");  
}  
  
module_init(hello_init);  
module_exit(hello_exit);  

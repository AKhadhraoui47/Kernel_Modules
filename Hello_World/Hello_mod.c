#include <linux/init.h>
#include <linux/module.h> 
#include <linux/types.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/printk.h> 

MODULE_LICENSE("GPL v3");
MODULE_AUTHOR("AKhadhraoui47");
MODULE_DESCRIPTION("Hello World Module");

static uint32_t argc_k = 0;
static uint16_t argv_k[2] = {-1, -1};
module_param_array(argv_k, short, &argc_k, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP );
MODULE_PARM_DESC(argv_k, "My Array of short Integers");

static int __init hello_init(void)
{
    uint8_t i = 0;
    pr_info("Hello, world\n");
    for (i = 0; i < ARRAY_SIZE(argv_k); i++)
    {
        pr_info("My argument [%hd] = %hd\n", i, argv_k[i]);
    }
    pr_info("Nb of arguments is %u\n", argc_k);
    return 0;
}

static void __exit hello_exit(void)
{
    pr_info("Goodbye, world\n");
}  

module_init(hello_init);
module_exit(hello_exit);


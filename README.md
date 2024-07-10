# Kernel_Modules

This repo will follow me learning Kernel Modules development from scratch wrapping things up with modules for Bluetooth/Wi-Fi communication devices. 

>  Many concepts are related to Kernel Modules and Kernel Development so we will start by understanding some key elements and their interaction with these modules before delving into coding.

#### User Space:

User space is the memory area where user applications run with restricted privileges, isolated from the core operating system and hardware interactions managed by the kernel with which the user space applications interact through **System Calls**.

#### System Calls:

System calls are the interface between user space applications and the kernel, allowing programs to request services such as file operations, memory management and process control by transferring control from unprivileged user mode to privileged kernel mode running in the **Kernel Space**.

#### Kernel Space:

Kernel space is the area of system memory reserved for the kernel, it is where the operating system core manages critical tasks like process management, memory allocation, and hardware communication through **Device Drivers** and **Kernel Modules**.

#### Device Drivers vs Kernel Modules:

A device driver is a kernel module that forms a software interface to an input/output (I/O) device. While kernel drivers are an integral part of the kernel, kernel modules offer a more flexible approach being capable to be loaded and unloaded to extend the kernel functionalities.

<sub>This diagram shows the main components we came across and the interactions between them to ensure stability and security of the our system</sub>  

![diagram](https://www.form3.tech/_prismic-media/e28ac8c54b950dd43cc9a62f49e76452445afdd054a5cab1a0e76b17b319ff89.png?auto=compress,format)  

<sub>**Source**: [Form3-Tech Linux Fundamentals](https://www.form3.tech/blog/engineering/linux-fundamentals-user-kernel-space)</sub>  

## Our First Module :monocle_face:  

As **Kernel Modules** run in the kernel space, the core of the operating system, writing their code is a delicate task that must ensure seamless interaction with the **Kernel**. This interaction is facilitated by ***Linux headers***, which are files containing necessary declarations, macros, constants, and function prototypes for developing both kernel-level and user-space applications. These ***Headers*** are essential for ensuring that code can properly interact with the Linux kernel and its various subsystems.  

> **Linux kernel headers** are essential files for developing and compiling kernel modules. These headers are part of the kernel but are shipped separately, so you need to install the **package version** that matches your target **kernel version**.   

<sub>As i am working on Ubuntu System</sub>

```console  
ak47@ak47:~$ sudo apt-install linux-headers-$(uname -r)  
```   

After installing the **Linux Headers** package you should find them under **/usr/src/linux-headers-$(target version)/include/** directory. Let's have a look at some well-known headers:  

+ **<linux/fs.h>** File system structures and functions.   
+ **<linux/printk.h>** Kernel messages logging.
+ **<linux/module.h>** Module-related macros and functions.     

> The **/usr/src/linux-headers-xxxxx/include** headers are used via the **/lib/modules/$(uname -r)/build** symbolic links. They are owned by the kernel headers packages and updated in lockstep with the kernel.

### Module Source Code Structure

The core of any kernel module is its source code written in C. This source file must include specific headers, particularly <linux/module.h>, which provides the necessary interfaces for module development.  

+ **Initialization function**  

The first function to be executed when a module is inserted is the initialization function which can be responsible for allocating resources and registering interfaces. After developing your function, you should declare it as a **Module Init Function** through the macro-like-function **module_init(x)**.  

+ **Exit function**  

When removing your module an exit function is executed to free memory previously allocated and other resources. After developing your function, you should declare it as a **Module Exit Function** through the macro-like-function **module_exit(x)**.  

+ **Licensing**  

For determining later usage scope of the module by any other user a license should be attributed to the module. THis can be achieved through the **MODULE_LICENSE** macro.  

To get a deeper understanding of these concepts and tools let's dive into our [Hello World module](Hello_World/Hello_mod.c) and clarify its composition.  

### Hello World :wave:  

Let's breakthrough our [Hello World module](Hello_World/Hello_mod.c) and explain its elements one by one.  

```
#include <linux/init.h>  /* Initializati on macros */
#include <linux/module.h>   /* Module development functions and macros */
#include <linux/moduleparam.h>  /* Module parameterizing from Cmd */
#include <linux/kernel.h>  /* Kernel core functions */
#include <linux/printk.h>  /* Kernel message logging */
```  
> Kernel message logging is an essential element for debbuging enhanced by **logging levels** macros.   

These are the **headers** that will provide us with the macros, functions for our module development.  

```  
MODULE_LICENSE("LICENSE");
MODULE_AUTHOR("Author name <email>");
MODULE_DESCRIPTION("Module description");
```  

These macros provided by **<linux/module.h>** are used for general information about the module. Note that mentionning the **License** is required.  

```
static unsigned int argc_k = 0;
static short int argv_k[2] = {-1, -1};
module_param_array(argv_k, short, &argc_k, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP );
MODULE_PARM_DESC(argv_k, "My Array of short Integers"); 
```  

This section of the code is responsible for setting the command line parameters, that we're familiar with as **argv** and **argc** that's i named them **argc_k** and **argv_k** as a reference to the kernel. After declaring our variables we should set the **argv_k** as a module parameter via **module_param_array()** macro provided by <linux/moduleparam.h>. **MODULE_PARM_DESC()** used to provide a description of the parameter variable.  

> module_param_array( **array_pointer**, **type**, **&countCmdParams**, **permissions**) 

> Different functions are provided for different params types; **module_param_string()** for strings, **module_param()** for int/long/char  

```  
static int __init hello_init(void)
{
    short int i = 0;
    pr_info("Hello, world\n");
    for (i = 0; i < ARRAY_SIZE(argv_k); i++)
    {
        pr_info("My argument [%hd] = %hd\n", i, argv_k[i]);
    }
    pr_info("Nb of arguments is %u\n", argc_k);
    return 0;
}
```  

This is the initialization function we have mentionned earlier which will be executed once, the moment our module is inserted. It's a **static** function as we don't need it outside of this file. The **pr_info()** function is kernel message logging with **info** loglevel, it's equivalent to **printk(KERN_INFO "msg");**.  

> **ARRAY_SIZE** macro-like-function provided by **<linux/kernel.h>** returning the size of the array.  

> **__init** macro causes the init function to be discarded and its memory freed once the init function finishes for built-in drivers, but not loadable modules.

```  
static void __exit hello_exit(void)
{
    pr_info("Goodbye, world\n");
}
```  

This is the exit function executed when the module is unloaded.  

> **__exit** macro causes the omission of the function when the module is built into the kernel, has no impact on loadable modules.  

```
module_init(hello_init);
module_exit(hello_exit);  
```  

These macro-like-function sets the **hello_init()** and **hello_exit()** as initialization and exit functions of our loadable module.  

So now that we explained and got familiar with our [Hello World module](Hello_World/Hello_mod.c) let's **make** it a loadable binary in our kernel.  

### Makefile  

<sub>To have a deep understanding of the basic concepts and structure of a **Makefile** you can check this [repository](https://github.com/AKhadhraoui47/Yocto_Rpi_IMU#makefile).</sub>  

To **make** our source code loadable as a **kernel object (.ko)** we will write a simple [Makefile](Hello_World/Makefile) that will get things done for us. So let's understand its structure:  

```
obj-m += Hello_mod.o
PWD := $(CURDIR)  
```  

**obj-m** registers the object files needed to generate the **kernel object**.
**PWD** registers our current directory.  

```
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

```  

This rule will have few steps before making our source code.   

1. Changing Directory to **/lib/modules/$(shell uname -r)/build** due to **-C** option.
2. Indicates our Source Code's path **M=$(PWD)**.
3. Executes the **modules** target.  

After the execution of this command we should be provided with our **Hello_mod.ko** loadable to our kernel.  

```  
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) cleanmake

```  

**cleanmake** is the target passed to make to clean up any previously built files, effectively preparing the directory for a fresh build.  

>  

Time to test our module.  

### Module Insertion  

For Loading and Unloading Modules:

+    **insmod**: Command to load a module into the kernel.
+    **rmmod**: Command to remove a module from the kernel.
+    **lsmod**: Lists currently loaded modules.

Let's insert our module

```console
ak47@ak47:~$ sudo insmod Hello_mod.ko argv_k=2,7
```  

> Problems as insertion not permitted are mostly related to **Secure Boot** being enabled so consider disabling it.  

Let's check our module is inserted:  
  
```console
ak47@ak47:~$ lsmod 
Module                  Size  Used by
Hello_mod              16384  0
```  

Let's check our init kernel messages:  

```console
ak47@ak47:~$ dmesg | tail -4
[18762.681253] Hello, world
[18762.681254] My argument [0] = 2
[18762.681255] My argument [1] = 7
[18762.681255] Nb of arguments is 2
```  

Now let's unload our module:  

```console
ak47@ak47:~$ sudo rmmod Hello_mod
ak47@ak47:~$ dmesg | tail -1
[18821.686439] Goodbye, world
```  

And now that we got familiar with modules development let's get to the real deal 

### :rocket:



## References :label:

[The Linux Kernel Module Programming Guide](https://sysprog21.github.io/lkmpg/)  <sub>Peter Jay Salzman, Michael Burian, Ori Pomerantz, Bob Mottram, Jim Huang</sub>  
  
[LINUX DEVICE DRIVERS 3rd Edition](https://www.google.com/url?sa=t&source=web&rct=j&opi=89978449&url=https://lwn.net/Kernel/LDD3/&ved=2ahUKEwjJ4vzx3JyHAxWjT6QEHZg9BcQQFnoECBQQAQ&usg=AOvVaw01bM6Zgwp5iRGPE8AVMxj-) <sub>Jonathan Corbet, Alessandro Rubini, and Greg Kroah-Hartman </sub>










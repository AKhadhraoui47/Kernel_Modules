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

For determining later usage scope of the module by any other user a license should be attributed to the module. THis can be achieved through the **MODULE_LICANSE** macro.  

To get a deeper understanding of these concepts and tools let's dive into our (Hello World module)[Hello_World/Hello_mod.c] and clzrify its composition.  

### Hello World :wave:  






 










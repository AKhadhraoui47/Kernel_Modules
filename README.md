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

<sub>This diagram shows the main components we have come through and the interactions between them in order to ensure stability and security of the our system</sub>

![diagram](https://www.form3.tech/_prismic-media/e28ac8c54b950dd43cc9a62f49e76452445afdd054a5cab1a0e76b17b319ff89.png?auto=compress,format)
<sub>**Source**: [Form3-Tech Linux Fundamentals](https://www.form3.tech/blog/engineering/linux-fundamentals-user-kernel-space)</sub>








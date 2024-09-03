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

The core of any kernel module is its source code written in C. This source file must include specific headers, particularly **<linux/module.h>**, which provides the necessary interfaces for module development.  

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

This section of the code is responsible for setting the command line parameters, that we're familiar with as **argv** and **argc** that's i named them **argc_k** and **argv_k** as a reference to the kernel. After declaring our variables we should set the **argv_k** as a module parameter via **module_param_array()** macro provided by **<linux/moduleparam.h>**. **MODULE_PARM_DESC()** used to provide a description of the parameter variable.  

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

And now that we got familiar with modules development let's get to the real deal, where we will be developing a kernel module for [**Grove Wifi V2 UART**](https://wiki.seeedstudio.com/Grove-UART_Wifi_V2/), so before moving we will need to test our module and understand how it works with [**AT Commands**](http://bbs.espressif.com/download/file.php?id=450).  

## Grove Wifi v2 Module :cloud:  

The Grove UART WiFi V2 is a serial transceiver module featuring the ESP8285 IoT SoC. It allows microcontrollers like Arduino to interact with WiFi networks using simple [**AT Commands**](http://bbs.espressif.com/download/file.php?id=450). So let's go through some basic commands which will be very useful in our module:

+ **AT**: Test the setup function of our module ( Response **OK** )
+ **AT+ATE0**: Disable commands echo from module to shorten received response ( Response **OK** )
+ **AT+SLEEP**: Manage the module state and power consumption ( Response **OK** )
+ **AT+UART_CUR**: Configure UART communication properties; baudrate, databits,stopbits, parity, flow control ( Response **OK** )  
+ **AT+CWMODE**: Set the module mode; Station, SoftAP, Station+SoftAP ( Response **OK** )  

So now after having two key elements ; The basics of Kernel Modules development and the basic commands to configure our module let's take a look at another important element which will allow us establish serial communication the [**<linux/serdev.h>**](https://github.com/torvalds/linux/blob/master/include/linux/serdev.h) kernel header.  

### Serdev.h :electric_plug:  

The **serdev.h** header provides a set of functions and macros to facilitate the management of serial devices within the Linux kernel. Here is an overview of its main functionnalities:  

+ **serdev_device_set_drvdata()**: Associates custom driver data with a serial device allowing driver to store and retrieve private data associated with the serial device.  
+ **serdev_device_set_client_ops()**: Assigns a set of operations (callbacks) to a serial device. 
+ **serdev_device_set_baudrate()**: sets the baud rate (the speed of communication) for the serial device. 
+ **serdev_device_set_flow_control()**: Enables or Disables flow control on the serial device.
+ **serdev_device_set_parity()**: Sets the parity mode for the serial device.
+ **serdev_device_write()**: Sends data to the serial device. It writes a specified number of bytes from a buffer to the device, with an optional timeout.
+ **struct serdev_device_driver**:  Represents a driver for a serial device within the serdev subsystem. It includes information about the driver, such as its name and probe and remove functions.
+ **struct serdev_device_ops**: defines callbacks that a serial device driver can implement. It includes function pointers for handling various events, such as data reception (**receive_buf**).  

Now let's go through our module [grovewifiv2](grovewifiv2/grovewifiv2.c) and understand its structure and functionnalities.  

> Keep in mind that this module was built for an **in-tree** module as a sub-node with UARTn that's why no **serdev_controller** was invoked. For more portability and scalabity for this module, improvements will be made.

### Grovewifiv2.c  

This kernel module is designed to interface with the Grove WiFi module from Seeed Studio, providing a structured way to manage and control the module using the Linux kernel's serdev subsystem.  

> Note that any variable or reference to **Command Line Interface** variable or function will be exploited later for a **CLI** to interact with this module in a user-friendly way from **user-space**. 

 #### 1. Linux Headers  

 ```
#include <linux/init.h> 
#include <linux/module.h>
#include <linux/serdev.h> 
#include <linux/of_device.h> 
#include <linux/completion.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
 ```  

 + **<linux/serdev.h>**: This header provides the necessary functions and structures to interact with the serdev subsystem.  
 + **<linux/of_device.h>**: Used for device tree matching, which allows the module to be bound to the Grove WiFi module based on its compatible string
 + **<linux/completion.h>**: Provides synchronization primitives to manage the completion of operations, such as waiting for a response from the WiFi module.
 + **<linux/mutex.h>**: Used to protect shared data structures from concurrent access, ensuring that communication with the WiFi module is thread-safe.
 + **<linux/sysfs.h> ; <linux/kobject.h>**: These headers are used to create sysfs entries, allowing user-space interaction with the kernel module.  

 #### 2. Module Specific Structure  

 ```
 struct grove_wifi_state {
    struct serdev_device *serdev;
    struct completion cmd_done;
    struct mutex lock;
    char response[GROVEWIFI_RSP_RAW_MAX_LENGTH];
    size_t response_len;
    enum grove_wifi_comm_state comm;
    struct kobject *kern_obj; //TODO
};
 ```

It's recommended to have a driver specific structure that holds specific data. Our custom-defined data structure that holds the state and relevant information needed to manage the Grove WiFi module within the kernel module encapsulating the state of the device, synchronization mechanisms, communication buffers.  

#### 3. Commands Table  

```
static u8 grove_wifi_cmd_tbl[][GROVE_CMD_MAX_LENGTH] = {
	[CMD_TEST] = "\r\nAT\r\n",
	[CMD_NO_ECHO] = "\r\nATE0\r\n",
	[CMD_DISABLE_SLEEP] = "\r\nAT+SLEEP=0\r\n",
	[CMD_SET_UART] = "\r\nAT+UART_CUR=115200,8,1,0,0\r\n",
	[CMD_STATION_MODE] = "\r\nAT+CWMODE=1\r\n",
    [CMD_CLI] = "\r\n\r\n",
};
```  

This table holds the configuration commands executed during the initialization, the last string **grove_wifi_cmd_tbl[CMD_CLI]** will hold the custom command for further setup. 

#### 4. Serial Device Operations  

```
static const struct serdev_device_ops grove_wifi_serdev_ops = {
    .receive_buf = grove_wifi_receive_buf,
    .write_wakeup = serdev_device_write_wakeup,
};
```

The **serdev_device_ops** structure holds two key callbacks; **receive_buf**  invoked whenever the kernel receives data from the Grove WiFi module through the serial interface, **write_wakeup** called when the device is ready to transmit more data after a write operation.  

#### 5. Open Firmware Device ID  

```
static const struct of_device_id grove_wifi_of_match[] = {
    { .compatible = "seeedstudio,grovewifiv1" },
	{ .compatible = "seeedstudio,grovewifiv2" },
	{ }
};
MODULE_DEVICE_TABLE(of, grove_wifi_of_match);
```  

The **of_device_id** structure is used in Linux kernel modules to define a table of device tree compatible strings that the driver can support.  **MODULE_DEVICE_TABLE** macro allows the kernel to create the necessary data structures so that when a device is detected with a compatible string, the corresponding driver is automatically loaded and bound to the device. <sub> Reminder we are working on an in-tree device </sub>    

#### 6. Driver properties

```
static struct serdev_device_driver grove_wifi_driver = {
    .driver = {
        .name = GROVE_WIFI_DRIVER_NAME,
        .of_match_table = of_match_ptr(grove_wifi_of_match),
    },
    .probe = grove_wifi_probe,
};
module_serdev_device_driver(grove_wifi_driver);
```  

The **driver** attribute is a general structure used by all types of device drivers in the kernel. **probe** attribute is a function pointer that points to the driver's probe function called by the kernel when a device that matches the driver's compatible string is found. **module_serdev_device_driver** This macro is used to register the serdev_device_driver with the kernel.  

> **module_serdev_device_driver** handles the module's initialization and cleanup functions (__init and __exit). So if you want to redefine them yourself expect errors will occur during compilation.  

#### 7. Module Structure

The functionnalities provided and the interactions between is explained as follows:   

###### a. Initialization

    module_serdev_device_driver(grove_wifi_driver)
Role: Registers the driver with the kernel.
Interaction: Calls the grove_wifi_probe() function when a matching device is found.

    grove_wifi_probe(struct serdev_device *serdev)
**Role**: Initializes the device, sets up serial communication, configures the device, and sets up sysfs entries.
**Interaction**: Calls several other functions:  
+ serdev_device_set_drvdata(serdev, state): Stores the device-specific data.  
+ serdev_device_set_client_ops(serdev, &grove_wifi_serdev_ops): Registers the serdev operations (grove_wifi_serdev_ops).  
+ serdev_device_set_baudrate(serdev, 115200), serdev_device_set_flow_control(serdev, false), and serdev_device_set_parity(serdev, SERDEV_PARITY_NONE): Configure the serial communication settings.  
+ grove_wifi_sysfs_setup(serdev): Creates sysfs entries for CLI and response handling.  

###### b. Sysfs Interface

    grove_wifi_sysfs_setup(struct serdev_device *serdev)
**Role**: Sets up sysfs entries for user interaction.  
**Interaction**: Calls sysfs_create_file() to create the files:  
+ id: Handles CLI ID input.  
+ cli: Handles command input.  
+ response: Displays the response status.  

**Sysfs Attribute Functions**:  
+ grove_wifi_fct_id_store(): Stores CLI ID.  
+ grove_wifi_cli_store(): Receives commands from the user and executes them via grove_wifi_do_cmd().  
+ grove_wifi_response_show(): Shows the status of the last command.  

###### c. Command Handling

    grove_wifi_do_cmd(struct grove_wifi_state *state, enum grove_wifi_cmd cmd)
**Role**: Sends a command to the Grove WiFi module and waits for a response.  
**Interaction**:  **Calls**:  
**grove_wifi_clear_frame(state)**: Clears the response buffer.  
**serdev_device_write(serdev, grove_wifi_cmd_tbl[cmd], ..., STD_TIMEOUT)**: Sends the command over serial.  
**wait_for_completion_timeout(&state->cmd_done, EXT_TIMEOUT)**: Waits for the response.

    grove_wifi_clear_frame(struct grove_wifi_state *state)
**Role**: Clears the response buffer and resets the communication state.

###### d. Communication

    static const struct serdev_device_ops grove_wifi_serdev_ops
    grove_wifi_receive_buf(struct serdev_device *serdev, const u8 *buf, size_t size)  
  
**Role**: Receives data from the Grove WiFi module and processes the response.  
**Interaction**: Updates the response buffer in state and completes the command if an "OK" response is received.

+ serdev_device_write_wakeup(struct serdev_device *serdev)
**Role**: (Typically not directly used) Wakes up the serial device after a write operation.

###### e. flow and Interaction

***Driver Loading***:
The kernel loads the module using module_serdev_device_driver, which registers the driver.
Upon detecting the Grove WiFi module, the kernel calls grove_wifi_probe.

***Device Initialization***:
grove_wifi_probe configures the serial communication and initializes the sysfs interface.
 
***Sysfs Interaction***:
Users interact with the device via sysfs. For example, writing to cli triggers a command to be sent using grove_wifi_do_cmd.

***Command Execution***:
grove_wifi_do_cmd sends the command and waits for a response.
The response is processed by grove_wifi_receive_buf, which updates the state and signals command completion.

***Status Check***:
Users can check the command status by reading from response.











## References :label:

[The Linux Kernel Module Programming Guide](https://sysprog21.github.io/lkmpg/)  <sub>Peter Jay Salzman, Michael Burian, Ori Pomerantz, Bob Mottram, Jim Huang</sub>  
  
[LINUX DEVICE DRIVERS 3rd Edition](https://www.google.com/url?sa=t&source=web&rct=j&opi=89978449&url=https://lwn.net/Kernel/LDD3/&ved=2ahUKEwjJ4vzx3JyHAxWjT6QEHZg9BcQQFnoECBQQAQ&usg=AOvVaw01bM6Zgwp5iRGPE8AVMxj-) <sub>Jonathan Corbet, Alessandro Rubini, and Greg Kroah-Hartman </sub>

[DigiKey](https://youtu.be/2-PwskQrZac?si=jiGgczR_U2r38A5k) <sub> Shawn Hymel </sub>

[Serial Device Bus](http://events17.linuxfoundation.org/sites/events/files/slides/serdev-elce-2017-2.pdf) <sub> Johan Hovold </sub>








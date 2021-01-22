
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>


#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/miscdevice.h>
#include <linux/spi/spi.h>

/* ----------------------------------------------- DRIVER hcsr --------------------------------------------------

 Basic driver example to show skelton methods for several file operations.

 ----------------------------------------------------------------------------------------------------------------*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/jiffies.h>

#include<linux/init.h>
#include<linux/moduleparam.h>

#include <linux/spinlock.h>
#include <linux/string.h>

#include <linux/proc_fs.h>
//#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <asm/msr.h>
#include <linux/spinlock_types.h>
#include <linux/unistd.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/ioctl.h>
#include <linux/spi/spi.h>











#define DEVICE_NAME "device_7219"

// Only create one spi device which corresponds to the LED matrix

static struct spi_board_info devs_7219 = {
	// Device name
	.modalias = DEVICE_NAME,
	// speed of transfer (10MHz -> 10^7 Hz)
	//.max_speed_hz = 1 * 1000000,
	.max_speed_hz = 1000 * 1000,
	// set on bus 1
	.bus_num = 1,
	// select 1 since 0 is being used by another spi device (will be changed later in driver)
	.chip_select = 1,
	// default mode
	.mode = 0,
};

// Need to get a pointer to the spi master of bus 1
static struct spi_master* spi_master_01;
// The led device to be added
static struct spi_device* device_7219;



static int p_device_init(void)
{
	int ret = 0;
	// Get SPI Master
	spi_master_01 = spi_busnum_to_master((uint16_t) 1);
	if(spi_master_01 == NULL) {
		printk("ERROR: Could not find spi master\n");
		return -1;
	}

	// Register SPI Device
	device_7219 = spi_new_device(spi_master_01, &devs_7219);
	if(device_7219 == NULL) {
		printk("ERROR: Unregistered device\n");
		return -1;
	}

	printk("SUCCESS: Registered device_7219\n");
	return ret;

}


static void p_device_exit(void)
{
	spi_unregister_device(device_7219);
	printk("SUCCESS: Unregistered device_7219\n");
	return;
}

module_init(p_device_init);
module_exit(p_device_exit);
MODULE_LICENSE("GPL");
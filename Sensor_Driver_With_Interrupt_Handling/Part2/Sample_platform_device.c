/*
 * A sample program to show the binding of platform driver and device.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include<linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/sysfs.h>



#include "Sample_platform_device.h"

struct list_head device_list;
LIST_HEAD(device_list);


static int NUMBER_OF_DEVICES = 2;
module_param(NUMBER_OF_DEVICES,int,0000);

static void torelease(struct device* device) {
	return;

}

/*
static struct P_chip P1_chip = {
		.name	= "xyz01",
		.dev_no 	= 1,
		.plf_dev = {
			.name	= "plat_1",
			.id	= -1,
			.dev = {.release = torelease,}

		}
};

static struct P_chip P2_chip = {
		.name	= "xyz02",
		.dev_no 	= 2,
		.plf_dev = {
			.name	= "plat_2",
			.id	= -1,
			.dev = {.release = torelease,}

		}
};
*/
/*
static struct P_chip P3_chip = {
		.name	= "xyz03",
		.dev_no 	= 3,
		.plf_dev = {
			.name	= "plat_3",
			.id	= -1,
			.dev = {.release = torelease,}

		}
};

static struct P_chip P4_chip = {
		.name	= "xyz04",
		.dev_no 	= 4,
		.plf_dev = {
			.name	= "plat_4",
			.id	= -1,
			.dev = {.release = torelease,}

		}
};

static struct P_chip P5_chip = {
		.name	= "xyz05",
		.dev_no 	= 5,
		.plf_dev = {
			.name	= "plat_5",
			.id	= -1,
			.dev = {.release = torelease,}

		}
};
*/



/*
static struct P_chip P3_chip = {
		.name	= "xyz03",
		.dev_no 	= 55,
		.plf_dev = {
			.name	= "defg",
			.id	= -1,
			.dev = {.release = torelease,}

		}
};
*/

#define DEV1 "plat_1"
#define DEV2 "plat_2"
#define DEV3 "plat_3"
#define DEV4 "plat_4"
#define DEV5 "plat_5"
#define DEV6 "plat_6"

static int p_device_init(void)
{
	int ret = 0;
	int i;
	struct P_chip* chip;

	

	printk("INFO: NUMBER_OF_DEVICES=%d\n", NUMBER_OF_DEVICES);
	if(NUMBER_OF_DEVICES <= 0) {
		return -1;
	}

	if(NUMBER_OF_DEVICES > 6) {
		NUMBER_OF_DEVICES = 6;
	}

	//char name[7] = {'p', 'l', 'a', 't', '_', '1', '\0'};
	for(i=0; i < NUMBER_OF_DEVICES; i++) {
		// Allocate new P_chip for each iteration

		printk("VAR: i=%d\n", i);

		printk("TRY: Allocate New Chip\n");
		chip = kmalloc(sizeof(struct P_chip), GFP_KERNEL);
		if(!chip) {
			printk("ERROR: Kmalloc failed for %d\n", i);
		}
		memset(chip, 0, sizeof(struct P_chip));

		
		if(i == 0) {
			printk("TRY: Set fields for i=0\n");
			chip->name = DEV1;
			printk("TRY: Set plfdev\n");
			chip->plf_dev.name = DEV1;

		}
		else if(i == 1) {
			chip->name = DEV2;
			chip->plf_dev.name = DEV2;

		}
		else if(i == 2) {
			chip->name = DEV3;
			chip->plf_dev.name = DEV3;
		}
		else if(i == 3) {
			chip->name = DEV4;
			chip->plf_dev.name = DEV4;

		}
		else if(i == 4) {
			chip->name = DEV5;
			chip->plf_dev.name = DEV5;
		}
		else {
			chip->name = DEV6;
			chip->plf_dev.name = DEV6;
		}

		
		printk("TRY: Set more device fields\n");
		chip->dev_no = i;
		chip->plf_dev.id = -1;
		chip->plf_dev.dev.release = torelease;
		printk("TRY: Register device\n");
		platform_device_register(&chip->plf_dev);	
		printk("TRY: Add to list\n");
		list_add_tail(&(chip->list_ptr), &device_list);

	}
	
	

	return ret;
}


static void p_device_exit(void)
{

	struct P_chip* device;
	struct P_chip* tmp;
	
	list_for_each_entry_safe(device, tmp, &device_list, list_ptr) {

		printk("TRY: Found Device\n");
    	platform_device_unregister(&device->plf_dev);
    	memset(device, 0, sizeof(struct P_chip));
    	kfree(device);

	}
	printk(KERN_ALERT "Goodbye, unregister the device\n");
}

/**
 * register the device when module is initiated
 */
/*
static int p_device_init(void)
{
	int ret = 0;
	int i;

	LIST_HEAD(DEVICE_LIST);

	
	

	// Register the device 
	platform_device_register(&P1_chip.plf_dev);
	
	printk(KERN_ALERT "Platform device 1 is registered in init \n");

	platform_device_register(&P2_chip.plf_dev);

	printk(KERN_ALERT "Platform device 2 is registered in init \n");

	char name[7] = {'H', 'C', 'S', 'R', '_', '1', '\0'};

	printk("INFO: NUMBER_OF_DEVICES=%d\n", NUMBER_OF_DEVICES);
	if(NUMBER_OF_DEVICES <= 0 || NUMBER_OF_DEVICES > 9) {
		return -1;
	}

	
	
	

	return ret;
}
*/
/*
static void p_device_exit(void)
{
    platform_device_unregister(&P1_chip.plf_dev);

	platform_device_unregister(&P2_chip.plf_dev);

	printk(KERN_ALERT "Goodbye, unregister the device\n");
}
*/
module_init(p_device_init);
module_exit(p_device_exit);
MODULE_LICENSE("GPL");

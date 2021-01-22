/* ----------------------------------------------- DRIVER kbuf --------------------------------------------------
 
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

#define DEVICE_NAME                 "kbuf"  // device name to be created and registered

/* per device structure */
struct kbuf_dev {
	struct cdev cdev;               /* The cdev structure */
	char name[20];                  /* Name of device*/
	char in_string[256];			/* buffer for the input string */
	int current_write_pointer;
} *kbuf_devp;

static dev_t kbuf_dev_number;      /* Allotted device number */
struct class *kbuf_dev_class;          /* Tie with the device model */
static struct device *kbuf_dev_device;

static char *user_name = "Dear John";  /* the default user name, can be replaced if a new name is attached in insmod command */

module_param(user_name,charp,0000);	//to get parameter from load.sh script to greet the user

/*
* Open kbuf driver
*/
int kbuf_driver_open(struct inode *inode, struct file *file)
{
	struct kbuf_dev *kbuf_devp;

	/* Get the per-device structure that contains this cdev */
	kbuf_devp = container_of(inode->i_cdev, struct kbuf_dev, cdev);


	/* Easy access to cmos_devp from rest of the entry points */
	file->private_data = kbuf_devp;
	printk("\n%s is openning \n", kbuf_devp->name);
	return 0;
}

/*
 * Release kbuf driver
 */
int kbuf_driver_release(struct inode *inode, struct file *file)
{
	struct kbuf_dev *kbuf_devp = file->private_data;
	
	printk("\n%s is closing\n", kbuf_devp->name);
	
	return 0;
}

/*
 * Write to kbuf driver
 */
ssize_t kbuf_driver_write(struct file *file, const char *buf,
           size_t count, loff_t *ppos)
{
	struct kbuf_dev *kbuf_devp = file->private_data;
	
	while (count) {	
		get_user(kbuf_devp->in_string[kbuf_devp->current_write_pointer], buf++);
		count--;
		if(count){
			kbuf_devp->current_write_pointer++;
			if( kbuf_devp->current_write_pointer == 256)
				kbuf_devp->current_write_pointer = 0;
		}
	}
	printk("Writing -- %d %s \n", kbuf_devp->current_write_pointer, kbuf_devp->in_string);
	return 0;
}
/*
 * Read to kbuf driver
 */
ssize_t kbuf_driver_read(struct file *file, char *buf,
           size_t count, loff_t *ppos)
{
	int bytes_read = 0;
	struct kbuf_dev *kbuf_devp = file->private_data;
	/*
	 * If we're at the end of the message, 
	 * return 0 signifying end of file 
	 */
	if (kbuf_devp->in_string[0] == 0)
		return 0;

	/* 
	 * Actually put the data into the buffer 
	 */
	while (count && kbuf_devp->in_string[bytes_read]) {

		put_user(kbuf_devp->in_string[bytes_read], buf++);
		count--;
		bytes_read++;
	}
	printk("Reading -- '%s'\n",kbuf_devp->in_string);
	/* 
	 * Most read functions return the number of bytes put into the buffer
	 */
	return bytes_read;

}

/* File operations structure. Defined in linux/fs.h */
static struct file_operations kbuf_fops = {
    .owner		= THIS_MODULE,           /* Owner */
    .open		= kbuf_driver_open,        /* Open method */
    .release	= kbuf_driver_release,     /* Release method */
    .write		= kbuf_driver_write,       /* Write method */
    .read		= kbuf_driver_read,        /* Read method */
};

/*
 * Driver Initialization
 */
int __init kbuf_driver_init(void)
{
	int ret;
	int time_since_boot;

	/* Request dynamic allocation of a device major number */
	if (alloc_chrdev_region(&kbuf_dev_number, 0, 1, DEVICE_NAME) < 0) {
			printk(KERN_DEBUG "Can't register device\n"); return -1;
	}

	/* Populate sysfs entries */
	kbuf_dev_class = class_create(THIS_MODULE, DEVICE_NAME);

	/* Allocate memory for the per-device structure */
	kbuf_devp = kmalloc(sizeof(struct kbuf_dev), GFP_KERNEL);
		
	if (!kbuf_devp) {
		printk("Bad Kmalloc\n"); return -ENOMEM;
	}

	/* Request I/O region */
	sprintf(kbuf_devp->name, DEVICE_NAME);

	/* Connect the file operations with the cdev */
	cdev_init(&kbuf_devp->cdev, &kbuf_fops);
	kbuf_devp->cdev.owner = THIS_MODULE;

	memset(kbuf_devp->in_string, 0, 256);

	/* Connect the major/minor number to the cdev */
	ret = cdev_add(&kbuf_devp->cdev, (kbuf_dev_number), 1);

	if (ret) {
		printk("Bad cdev\n");
		return ret;
	}

	/* Send uevents to udev, so it'll create /dev nodes */
	kbuf_dev_device = device_create(kbuf_dev_class, NULL, MKDEV(MAJOR(kbuf_dev_number), 0), NULL, DEVICE_NAME);		

	time_since_boot=(jiffies-INITIAL_JIFFIES)/HZ;//since on some systems jiffies is a very huge uninitialized value at boot and saved.
	sprintf(kbuf_devp->in_string, "Hi %s, this machine has been on for %d seconds", user_name, time_since_boot);
	
	kbuf_devp->current_write_pointer = 0;

	printk("kbuf driver initialized.\n'%s'\n",kbuf_devp->in_string);
	return 0;
}
/* Driver Exit */
void __exit kbuf_driver_exit(void)
{
	// device_remove_file(kbuf_dev_device, &dev_attr_xxx);
	/* Release the major number */
	unregister_chrdev_region((kbuf_dev_number), 1);

	/* Destroy device */
	device_destroy (kbuf_dev_class, MKDEV(MAJOR(kbuf_dev_number), 0));
	cdev_del(&kbuf_devp->cdev);
	kfree(kbuf_devp);
	
	/* Destroy driver_class */
	class_destroy(kbuf_dev_class);

	printk("kbuf driver removed.\n");
}

module_init(kbuf_driver_init);
module_exit(kbuf_driver_exit);
MODULE_LICENSE("GPL v2");

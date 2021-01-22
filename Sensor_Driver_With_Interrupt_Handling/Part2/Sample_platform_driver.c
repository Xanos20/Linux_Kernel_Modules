/*
 * A sample program to show the binding of platform driver and device.
 */

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include "Sample_platform_device.h"
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include "hcsr.h"
#include <linux/sysfs.h>
#include <linux/uaccess.h>
#include <asm/msr.h>


#define DRIVER_NAME		"platform_driver_0"
#define DEVICE_NAME "PLAT_HCSR"


/*
static struct class_atrribute hcsr_class_attrs[] = {

};
*/

static struct class HCSR_CLASS = {
	.name  = "HCSR",
	.owner = THIS_MODULE,
	//.class_attrs = hcsr_class_attrs,
};

static dev_t dev_major = 12;

// Driver workqueue for all devices
static struct workqueue_struct* my_wq;


static const struct platform_device_id P_id_table[] = {
         { "plat_1", 0 },
         { "plat_2", 0 },
         { "plat_3", 0 },
         { "plat_4", 0 },
         { "plat_5", 0 },
         { "plat_6", 0 },
	 { },
};




//====================================================================================

// Determines whether to add a new class that each device created by module is initialized under
static bool class_enabled = false;


// The global list that holds each created hcsr_dev device
struct list_head device_list;
LIST_HEAD(device_list);

//====================================================================================


int hcsr_driver_open(struct inode *inode, struct file *file)
{
	struct hcsr_dev *hcsr_devps;

	/* Get the per-device structure that contains this cdev */
	//hcsr_devps = container_of(inode->i_cdev, struct hcsr_dev, cdev);

	
	int minor_number = iminor(inode);

	struct hcsr_dev* tmp;
	struct hcsr_dev* cursor;
	list_for_each_entry_safe(cursor, tmp, &device_list, list_ptr) {
		// Search for matching device
		printk("INFO: Found Device Minor Number = %d\n", cursor->misc.minor);
		if(cursor->misc.minor == minor_number) {
			hcsr_devps = cursor;
			printk("\n%s is openning \n", hcsr_devps->misc.name);
			return 0;
		}
	}

	printk("ERROR: Could not find device\n");
	return -1;

}

/*
 * Release hcsr driver
 */
int hcsr_driver_release(struct inode *inode, struct file *file)
{
	struct hcsr_dev *hcsr_devps = file->private_data;


	printk("\n%s is closing\n", hcsr_devps->name);

	return 0;
}





// TRIGGER PARSING ==================================================================================================

/*
	Set corresponding trigger pin sequence based on user chosen IO trigger pin
*/
long set_trigger_pins(struct hcsr_dev *hcsr_devps) {
	int ret;
	int io_trig = hcsr_devps->trigger;

	if(io_trig == 0) {
		printk("TRY: Request gpio11, gpio33\n");
		// 11
		ret = gpio_request(11, "gpio11");
		if(ret != 0) {
			printk("ERROR: gpio11\n");
			return -1;
		}
		ret = gpio_direction_output(11, 0);
		if(ret != 0) {
			printk("ERROR: gpio11 direction\n");
			return -1;
		}
		hcsr_devps->gpio_trigger = 11;
		// 33
		ret = gpio_request(33, "gpio33");
		if(ret != 0) {
			printk("ERROR: gpio33\n");
			return -1;
		}
		ret = gpio_direction_output(33, 0);
		if(ret != 0) {
			printk("ERROR: gpio33 output\n");
			return -1;
		}
	}

	else if(io_trig == 1) {
		printk("TRY gpio12, gpio28, gpio45\n");
		// 12
		ret = gpio_request(12, "gpio12");
		if(ret != 0) {
			printk("ERROR: gpio12\n");
			return -1;
		}
		ret = gpio_direction_output(12, 0);
		if(ret != 0) {
			printk("ERROR: gpio12 output\n");
			return -1;
		}
		hcsr_devps->gpio_trigger = 12;
		// 28
		ret = gpio_request(28, "gpio28");
		if(ret != 0) {
			printk("ERROR\n");
			return -1;
		}
		ret = gpio_direction_output(28, 0);
		if(ret != 0) {
			return -1;
		}
		// 45
		ret = gpio_request(45, "gpio45");
		if(ret != 0) {
			return -1;
		}
		ret = gpio_direction_output(45, 0);
		if(ret != 0) {
			return -1;
		}
	}

	else if(io_trig == 4) {
		printk("TRY: gpio6, gpio36\n");
		// 6
		ret = gpio_request(6, "gpio6");
		if(ret != 0) {
			return -1;
		}
		ret = gpio_direction_output(6, 0);
		if(ret != 0) {
			return -1;
		}
		hcsr_devps->gpio_trigger = 6;
		// 36
		ret = gpio_request(36, "gpio36");
		if(ret != 0) {
			return -1;
		}
		ret = gpio_direction_output(36, 0);
		if(ret != 0) {
			return -1;
		}
	}

	else if(io_trig == 5) {
		printk("TRY: gpio0, gpio18, gpio66\n");
		// 0
		ret = gpio_request(0, "gpio0");
		if(ret != 0) {
			return -1;
		}
		ret = gpio_direction_output(0, 0);
		if(ret != 0) {
			return -1;
		}
		hcsr_devps->gpio_trigger = 0;
		// 18
		ret = gpio_request(18, "gpio18");
		if(ret != 0) {
			return -1;
		}
		ret = gpio_direction_output(18, 0);
		if(ret != 0) {
			return -1;
		}
		// 66
		ret = gpio_request(66, "gpio66");
		if(ret != 0) {
			return -1;
		}
		gpio_set_value_cansleep(66, 0);
	}

	else if(io_trig == 6) {
		printk("TRY: gpio1, gpio20, gpio68\n");
		// 1
		ret = gpio_request(1, "gpio1");
		if(ret != 0) {
			return -1;
		}
		ret = gpio_direction_output(1, 0);
		if(ret != 0) {
			return -1;
		}
		hcsr_devps->gpio_trigger = 1;
		// 20
		ret = gpio_request(20, "gpio20");
		if(ret != 0) {
			return -1;
		}
		ret = gpio_direction_output(20, 0);
		if(ret != 0) {
			return -1;
		}
		// 68
		ret = gpio_request(68, "gpio68");
		if(ret != 0) {
			return -1;
		}
		gpio_set_value_cansleep(68, 0);
	}

	else if(io_trig == 7) {
		printk("TRY: gpio38\n");
		// 38
		ret = gpio_request(38, "gpio38");
		if(ret != 0) {
			gpio_free(38);
			ret = gpio_request(38, "gpio38");
			if(ret != 0) {
				printk("ERROR: gpio38\n");
				return -1;
			}
		}
		ret = gpio_direction_output(38, 0);
		if(ret != 0) {
			printk("ERROR: gpio38 direction out\n");
			return -1;
		}
		hcsr_devps->gpio_trigger = 38;
		printk("SUCCESS: gpio38\n");
	}

	else if(io_trig == 8) {
		printk("TRY: gpio40\n");
		// 40
		ret = gpio_request(40, "gpio40");
		if(ret != 0) {
			return -1;
		}
		ret = gpio_direction_output(40, 0);
		if(ret != 0) {
			return -1;
		}
		hcsr_devps->gpio_trigger = 40;
	} 

	else {

	}
	
	printk("SUCCESS: Trigger Pin setup\n");
	hcsr_devps->trigger_enabled = true;
	return 0;
}




// ECHO INTERRUPT AND PARSING ==============================================================================================


/*
 Assembly code to get the Time Stamp Counter
 */
static __always_inline unsigned long long get_native_read_tsc_here(void) {
	DECLARE_ARGS(val, low, high);
	asm volatile("rdtsc" : EAX_EDX_RET(val, low, high));
	return (unsigned long long) EAX_EDX_VAL(val, low, high);
}


/*
	INFO: the echo pin interrupt handler, should be initialized as a rising edge first 
	 	and should be activated first when echo pin becomes high in the thread function

	ACTION:		Should write the start and end time stamp counters of the sensor measurement into the internal buffer
*/

irqreturn_t irq_echo_handler(int irq, void* dev_id) {

	struct hcsr_dev* hcsr_devps;
	hcsr_devps = (struct hcsr_dev*) dev_id;

	//printk("INFO: In Interrupt\n");

	// LOCK: to prevent race conditions in the internal buffer and irq flag
	spin_lock_irqsave(&(hcsr_devps->lock_irq), hcsr_devps->lock_irq_flags);

	if(irq == hcsr_devps->irq_desc_echo) {
		// Verify this the correct irq number

		if(hcsr_devps->irq_is_rising) {
			// RISING EDGE

			//printk("INFO: RISING\n");
			// Write timestamp of start of sensor measurement
			hcsr_devps->cursor->tsc_rising = get_native_read_tsc_here();
			
			// Change trigger for falling edge
			irq_set_irq_type(irq, IRQ_TYPE_EDGE_FALLING);

			// update flag
			hcsr_devps->irq_is_rising = false;
		}
		else if(hcsr_devps->irq_is_rising == false) {
			// FALLING EDGE

			//printk("INFO: FALLING\n");
			// Write timestamp of start of sensor measurement
			hcsr_devps->cursor->tsc_falling = get_native_read_tsc_here();

			// Change irq type back to rising
			irq_set_irq_type(irq, IRQ_TYPE_EDGE_RISING);

			// update flag
			hcsr_devps->irq_is_rising = true;

		}
		else {
			// Ignore interrupt
		}

	}
	else {
		// Different IRQ Number
	}

	// UNLOCK
	spin_unlock_irqrestore(&(hcsr_devps->lock_irq),  hcsr_devps->lock_irq_flags);
	return IRQ_HANDLED;
}



/*
 Setup new interrupt for the gpio echo pin that is the first one for respective IO shield pin
*/
int setup_echo_pin_interrupt(struct hcsr_dev* hcsr_devps) {

	int verify_irq;

	// Request new interrupt line
	hcsr_devps->irq_desc_echo = gpio_to_irq(hcsr_devps->gpio_echo);
	if(hcsr_devps->irq_desc_echo < 0) {
		printk("ERROR=%d: gpio_to_irq\n", hcsr_devps->irq_desc_echo);
		return -1;
	}

	// Request interrupt for gpio_echo, the interrupt is first triggered at a rising edge (gpio_trig value goes from 0 to 1)
	verify_irq = request_irq(hcsr_devps->irq_desc_echo, (irq_handler_t) irq_echo_handler, IRQF_TRIGGER_RISING, hcsr_devps->misc.name, (void*) hcsr_devps);
	if(verify_irq != 0) {
		printk("ERROR=%d: request_irq", verify_irq);
		return -1;
	}

	printk("SUCCESS: Interrupt Requested\n");
	hcsr_devps->has_interrupt = true;
	return 0;

}




long set_echo_pins(struct hcsr_dev *hcsr_devps) {
	//int ver;
	int ret;
	//int c;
	int io_echo = hcsr_devps->echo;
	
	if(io_echo == 4) {
		printk("TRY: Setting up gpio6 & gpio36\n");
		// 6
		ret = gpio_request(6, "gpio6");
		if(ret != 0) {
			printk("ERROR: gpio6\n");
			return -1;
		}
		ret = gpio_direction_input(6);
		if(ret != 0) {
			printk("ERROR: gpio6 direction input\n");
			return -1;
		}
		// 36
		ret = gpio_request(36, "gpio36");
		if(ret != 0) {
			printk("ERROR: gpio36");
			return -1;
		}
		ret = gpio_direction_output(36, 1);
		if(ret != 0) {
			printk("ERROR: gpio36 direction out\n");
			return -1;
		}
		// For interrupts
		hcsr_devps->gpio_echo = 6;
	}

	else if(io_echo == 5) {
		printk("TRY: Setting up gpio0, gpio18, gpio66\n");
		// 0
		ret = gpio_request(0, "gpio0");
		if(ret != 0) {
			printk("ERROR: gpio0\n");
			return -1;
		}
		ret = gpio_direction_input(0);
		if(ret != 0) {
			printk("ERROR: gpio0 input\n");
			return -1;
		}
		// 18
		ret = gpio_request(18, "gpio18");
		if(ret != 0) {
			printk("ERROR: gpio18\n");
			return -1;
		}
		ret = gpio_direction_output(18, 1);
		if(ret != 0) {
			printk("ERROR: gpio18 direction\n");
			return -1;
		}
		// 66
		ret = gpio_request(66, "gpio66");
		if(ret != 0) {
			printk("ERROR: gpio66\n");
			return -1;
		}
		gpio_set_value_cansleep(66, 0);
		// For interrupts
		hcsr_devps->gpio_echo = 0;
	}

	else if(io_echo == 6) {
		printk("TRY: Setting up gpio1, gpio20, gpio68\n");
		// 1
		ret = gpio_request(1, "gpio1");
		if(ret != 0) {
			printk("ERROR: gpio1\n");
			return -1;
		}
		ret = gpio_direction_input(1);
		if(ret != 0) {
			printk("ERROR: gpio1\n");
			return -1;
		}
		// 20
		ret = gpio_request(20, "gpio20");
		if(ret != 0) {
			printk("ERROR: gpio20\n");
			return -1;
		}
		ret = gpio_direction_output(20, 1);
		if(ret != 0) {
			printk("ERROR: gpio20 direction\n");
			return -1;
		}
		// 68
		ret = gpio_request(68, "gpio68");
		if(ret != 0) {
			printk("ERROR: gpio68\n");
			return -1;
		}
		gpio_set_value_cansleep(68, 0);
		// For interrupts
		hcsr_devps->gpio_echo = 1;
	}

	else if(io_echo == 9) {
		printk("TRY: Request gpio4, gpio22, gpio70\n");
		// 4
		ret = gpio_request(4, "gpio4");
		if(ret != 0) {
			printk("ERROR: gpio4\n");
			return -1;
		}
		ret = gpio_direction_input(4);
		if(ret != 0) {
			printk("ERROR: gpio4 direction\n");
			return -1;
		}
		// 22
		ret = gpio_request(22, "gpio22");
		if(ret != 0) {
			printk("ERROR: gpio22\n");
			return -1;
		}
		ret = gpio_direction_output(22, 1);
		if(ret != 0) {
			printk("ERROR: gpio22 output\n");
			return -1;
		}
		// 70
		ret = gpio_request(70, "gpio70");
		if(ret != 0) {
			printk("ERROR: gpio70\n");
			return -1;
		}
		gpio_set_value_cansleep(70, 0);
		// For interrupt
		hcsr_devps->gpio_echo = 4;
	}

	else if(io_echo == 11) {
		// TODO
	}

	else if(io_echo == 13) {
		printk("TRY: Enable gpio7, gpio30, gpio46");
		// 7
		ret = gpio_request(7, "gpio7");
		if(ret != 0) {
			printk("ERROR: gpio7\n");
			return -1;
		}
		ret = gpio_direction_input(7);
		if(ret != 0) {
			printk("ERROR: gpio7 input\n");
			return -1;
		}
		// 30
		ret = gpio_request(30, "gpio30");
		if(ret != 0) {
			printk("ERROR: gpio30\n");
			return -1;
		}
		ret = gpio_direction_output(30, 1);
		if(ret != 0) {
			printk("ERROR: gpio30 output\n");
			return -1;
		}
		// 46
		ret = gpio_request(46, "gpio46");
		if(ret != 0) {
			printk("ERROR: gpio46\n");
			return -1;
		}
		ret = gpio_direction_output(46, 0);
		if(ret != 0) {
			printk("ERROR: gpio46 direction\n");
			return -1;
		}
		// For interrupt
		hcsr_devps->gpio_echo = 7;
	}

	else {
		printk("ERROR: Parse io_echo\n");
		return -1;
	}
	
	printk("SUCCESS: Setup Echo Pin\n");
	hcsr_devps->echo_enabled = true;
	return 0;

}









// Latest Distance ==========================================================================================

static ssize_t show_latest_distance(struct device* dev, struct device_attribute* attr, char* buf) {
	struct hcsr_dev* hcsr_devps;
	int status;
	hcsr_devps = dev_get_drvdata(dev);
	if(hcsr_devps->external.position == 0) {
		status = snprintf(buf, 4096, "Latest_Average_Distance=%llu\n",  hcsr_devps->external.buffer[hcsr_devps->external.position].distance);
	}
	else {
		status = snprintf(buf, 4096, "Latest_Average_Distance=%llu\n",  hcsr_devps->external.buffer[hcsr_devps->external.position-1].distance);

	}
	return status;
}


static struct device_attribute dev_attr_latest_distance = {
	.attr = {
		.name = "latest_distance",
		.mode = S_IWUSR | S_IRUGO,
	},
	.show = show_latest_distance,
};


// Enable Measurement =========================================================================================================







/*
	The work_queue function to activate interrupt by turning the trigger pin on and off
*/
void work_function(struct work_struct* work) {

	unsigned long long pulse_width;
	struct hcsr_dev* hcsr_devps;
	struct sample_tsc* tmp;
	struct sample_tsc* cursor;

	struct sample_tsc* temp;
	struct sample_tsc* curr;
	unsigned long long velocity = 340; 
	unsigned long long average_distance = 0;
	//int p = 0;

	
	//printk("TRY: Work Function\n");
	hcsr_devps = container_of(work, struct hcsr_dev, my_work);

	while(spin_is_locked(&(hcsr_devps->lock_ops)) == 1) {
		//printk("INFO: WAIT FOR LOCK\n");
		// Wait for lock
		msleep(10);
	}

	// LOCK
	spin_lock(&(hcsr_devps->lock_ops));

	hcsr_devps->take_meas = true;
	if(hcsr_devps->take_meas == true) {
		// Trigger Interrupts and take samples

		hcsr_devps->currently_sampling = true;

		printk("TRY: Work Function Take Measurements\n");
	
		// Update external buffer pointer to current position
		hcsr_devps->external.position = hcsr_devps->external.position % 5;

		// Make sure trigger pin is off before sampling since irq is rising first
		//gpio_set_value_cansleep(hcsr_devps->gpio_trigger, TRIG_PIN_OFF);

		list_for_each_entry_safe(cursor, tmp, &(hcsr_devps->samples_head), list_ptr) {
			//printk("TRY: Trigger\n");
			// Take measurements for cursor in irq handler
			hcsr_devps->cursor = cursor;
			// enable interrupt and take measurements for time period delta
			gpio_set_value_cansleep(hcsr_devps->gpio_trigger, TRIG_PIN_ON);
			// delay for echo time
			udelay((unsigned long) 100);
			// take last sample for time period
			gpio_set_value_cansleep(hcsr_devps->gpio_trigger, TRIG_PIN_OFF);
			// sleep for delta time
			msleep((unsigned int) hcsr_devps->sampling_period);
		}
		printk("INFO: Sampling Finished\n");
		
		list_for_each_entry_safe(curr, temp, &(hcsr_devps->samples_head), list_ptr) {
			// Calculate distances for each internal sample, add each valid distance to average, and get last timestamp
				
			pulse_width = curr->tsc_falling - curr->tsc_rising;
			//printk("VAR: Falling-Rising = %lld\n", pulse_width);
			//printk("VAR: Falling-Rising INT = %d\n", (int) pulse_width);
			pulse_width = pulse_width * velocity;
			//printk("VAR: Pulse Width = %llu\n", pulse_width);
			curr->distance = ((int) pulse_width) / 2;

			if(curr->order > 0 && curr->order < hcsr_devps->number_samples+1) {
				// Add to average distance
				average_distance += ((unsigned long long) curr->distance);
				//printk("VAR: Current Average Distance = %llu\n",  average_distance);
			}
			if(curr->order == hcsr_devps->number_samples+1) {
				// Get last timestamp
				hcsr_devps->external.buffer[hcsr_devps->external.position].tsc_last = curr->tsc_falling;
			}

		}
		
		average_distance = (unsigned long long) ( ((int) average_distance) / hcsr_devps->number_samples);
		printk("INFO: AVERAGE DISTANCE = %llu\n",  average_distance);
	
		// Set average distance
		hcsr_devps->external.buffer[hcsr_devps->external.position].distance = (unsigned long long) average_distance;
	
		// Update external buffer counter for next samples
		hcsr_devps->external.position += 1;
		
		// Do not take any more measurements until the write function changes take_meas to true
		hcsr_devps->take_meas = false;

	}
	else {
		msleep(10);
	}
	

	hcsr_devps->currently_sampling = false;
	// UNLOCK
	spin_unlock(&(hcsr_devps->lock_ops));
	return;

}




static ssize_t store_enable_measurements(struct device* dev, struct device_attribute* attr, const char* buf, size_t size) {
	struct hcsr_dev* hcsr_devps;
	long value;
	int enable;
	int status;
	hcsr_devps = dev_get_drvdata(dev);
	spin_lock(&(hcsr_devps->lock_ops));

	status = kstrtol(buf, 10, &value);
	if(status != 0) {
		printk("ERROR: store_sampling_period\n");
		spin_unlock(&(hcsr_devps->lock_ops));
		return -1;
	}

	enable = (int) value;

	
	if(hcsr_devps->currently_sampling == true) {
		printk("ERROR: Measurement In Progress\n");
		spin_unlock(&(hcsr_devps->lock_ops));
		return 4096;
	}

	if(hcsr_devps->trigger_enabled==false || hcsr_devps->echo_enabled==false) {
		printk("ERROR: Echo and/or Trigger Pins Not Enabled\n");
		spin_unlock(&(hcsr_devps->lock_ops));
		return 4096;

	}

	if(hcsr_devps->has_interrupt == false) {
		printk("ERROR: Missing interrupt\n");
		spin_unlock(&(hcsr_devps->lock_ops));
		return 4096;
	}


	if(enable == 1) {
		// Trigger interrupts and take measurements
		printk("TRY: Trigger Measurements\n");
		hcsr_devps->take_meas = true;
		//hcsr_devps->currently_sampling == true;
		INIT_WORK(&(hcsr_devps->my_work), work_function);
		status = queue_work(my_wq, &(hcsr_devps->my_work));
	}
	else {
		enable = 0;
	}

	
	spin_unlock(&(hcsr_devps->lock_ops));
	return 4096;
}




static ssize_t show_enable_measurements(struct device* dev, struct device_attribute* attr, char* buf) {
	struct hcsr_dev* hcsr_devps;
	int status;
	hcsr_devps = dev_get_drvdata(dev);
	//spin_lock(&(hcsr_devps->lock_ops));
	status = snprintf(buf, 4096, "Enable_Measurements=%d\n", hcsr_devps->take_meas);
	//spin_unlock(&(hcsr_devps->lock_ops));
	return status;
}



static struct device_attribute dev_attr_enable_measurements = {
	.attr = {
		.name = "enable_measurements",
		.mode = S_IWUSR | S_IRUGO,
	},
	.show = show_enable_measurements,
	.store = store_enable_measurements,
};


// Sampling Period ======================================================================================================



static ssize_t store_sampling_period(struct device* dev, struct device_attribute* attr, const char* buf, size_t size) {
	struct hcsr_dev* hcsr_devps;
	long value;
	int status;
	hcsr_devps = dev_get_drvdata(dev);

	spin_lock(&(hcsr_devps->lock_ops));

	status = kstrtol(buf, 10, &value);
	if(status != 0) {
		printk("ERROR: store_sampling_period input\n");
		spin_unlock(&(hcsr_devps->lock_ops));
		return -1;
	}

	if(((int) value) < 100 || ((int) value) > 500) {
		hcsr_devps->sampling_period = 100;
		spin_unlock(&(hcsr_devps->lock_ops));
		return 4096;
	}

	hcsr_devps->sampling_period = (int) value;

	spin_unlock(&(hcsr_devps->lock_ops));
	return 4096;
}

static ssize_t show_sampling_period(struct device* dev, struct device_attribute* attr, char* buf) {
	struct hcsr_dev* hcsr_devps;
	int status;
	hcsr_devps = dev_get_drvdata(dev);
	spin_lock(&(hcsr_devps->lock_ops));
	status = snprintf(buf, 4096, "Sampling_Period=%d\n", hcsr_devps->sampling_period);
	spin_unlock(&(hcsr_devps->lock_ops));
	return status;
}


static struct device_attribute dev_attr_sampling_period = {
	.attr = {
		.name = "sampling_period",
		.mode = S_IWUSR | S_IRUGO,
	},
	.show = show_sampling_period,
	.store = store_sampling_period,
};



// Number Of Samples (number_samples) ================================================================================

/*
static ssize_t make_internal_buffer(struct hcsr_dev* hcsr_devps) {
	int p = 0;
	for(; p < hcsr_devps->number_samples+2; p++) {
		// Create internal sample buffer which has two additional samples for first and last sample

		struct sample_tsc* sample;
		sample = kmalloc(sizeof(struct sample_tsc), GFP_KERNEL);
		if(!sample) {
			printk("ERROR: Kmalloc error in creating internal buffer\n");
			return -1;
		} 
		memset(sample, 0, sizeof(struct sample_tsc));
		sample->order = p;
		if(p == 0) {
			INIT_LIST_HEAD(&(hcsr_devps->samples_head));

		}
		// add to back of list
		list_add_tail(&(sample->list_ptr), &(hcsr_devps->samples_head));
		hcsr_devps->sample_list_size += 1;
	}
	return 0;

}
*/


ssize_t recreate_internal_buffer(struct hcsr_dev* hcsr_devps) {
	struct sample_tsc* cursor;
	struct sample_tsc* tmp;
	int p;

	if(hcsr_devps->currently_sampling == true) {
		printk("INFO: Currently Sampling\n");
		return 0;
	}

	// LOCK: to prevent race conditions in the internal buffer and irq flag
	spin_lock_irqsave(&(hcsr_devps->lock_irq), hcsr_devps->lock_irq_flags);

	list_for_each_entry_safe(cursor, tmp, &(hcsr_devps->samples_head), list_ptr) {
		list_del(&(cursor->list_ptr));
		memset(cursor, 0, sizeof(struct sample_tsc));
		kfree(cursor);
	}

	hcsr_devps->sample_list_size = 0;
	//printk("TRY: Create New Internal Buffer\n");

	for(p=0; p < hcsr_devps->number_samples+2; p++) {
		// Create internal sample buffer which has two additional samples for first and last sample
		struct sample_tsc* sample;
		sample = kmalloc(sizeof(struct sample_tsc), GFP_KERNEL);
		if(!sample) {
			printk("ERROR: Kmalloc error in creating internal buffer\n");
			// UNLOCK
			spin_unlock_irqrestore(&(hcsr_devps->lock_irq),  hcsr_devps->lock_irq_flags);
			return -1;
		} 
		memset(sample, 0, sizeof(struct sample_tsc));
		sample->order = p;
		// add to back of list
		list_add_tail(&(sample->list_ptr), &(hcsr_devps->samples_head));
		hcsr_devps->sample_list_size += 1;
	}

	printk("SUCCESS: Created Internal Buffer\n");
	// UNLOCK
	spin_unlock_irqrestore(&(hcsr_devps->lock_irq),  hcsr_devps->lock_irq_flags);
	return 0;
}



static ssize_t store_number_samples(struct device* dev, struct device_attribute* attr, const char* buf, size_t size) {
	struct hcsr_dev* hcsr_devps;
	long value;
	int status;
	//printk("TRY: Store Trigger\n");
	hcsr_devps = dev_get_drvdata(dev);

	spin_lock(&(hcsr_devps->lock_ops));

	status = kstrtol(buf, 10, &value);
	if(status != 0) {
		printk("ERROR: store_trigger input\n");
		spin_unlock(&(hcsr_devps->lock_ops));
		return -1;
	}
	
	hcsr_devps->number_samples = (int) value;
	//status = make_internal_buffer(hcsr_devps);
	status = recreate_internal_buffer(hcsr_devps);
	if(status != 0) {
		printk("ERROR: Create internal buffer\n");
		spin_unlock(&(hcsr_devps->lock_ops));
		return 4096;
	}

	spin_unlock(&(hcsr_devps->lock_ops));
	return 4096;
}

static ssize_t show_number_samples(struct device* dev, struct device_attribute* attr, char* buf) {
	struct hcsr_dev* hcsr_devps;
	int status;
	hcsr_devps = dev_get_drvdata(dev);
	spin_lock(&(hcsr_devps->lock_ops));
	status = snprintf(buf, 4096, "Number_Samples=%d\n", hcsr_devps->number_samples);
	spin_unlock(&(hcsr_devps->lock_ops));
	return status;
}


static struct device_attribute dev_attr_number_samples = {
	.attr = {
		.name = "number_samples",
		.mode = S_IWUSR | S_IRUGO,
	},
	.show = show_number_samples,
	.store = store_number_samples,
};



// Trigger ==========================================================================================================

static ssize_t show_trigger(struct device* dev, struct device_attribute* attr, char* buf) {
	struct hcsr_dev* hcsr_devps;
	int status;
	printk("TRY: In show_echo\n");
	hcsr_devps = dev_get_drvdata(dev);
	
	spin_lock(&(hcsr_devps->lock_ops));

	status = snprintf(buf, 4096, "Trigger IO=%d\n", hcsr_devps->trigger);

	spin_unlock(&(hcsr_devps->lock_ops));
	return status;
}


static ssize_t store_trigger(struct device* dev, struct device_attribute* attr, const char* buf, size_t size) {
	struct hcsr_dev* hcsr_devps;
	long value;
	int status;
	//printk("TRY: Store Trigger\n");
	hcsr_devps = dev_get_drvdata(dev);

	spin_lock(&(hcsr_devps->lock_ops));

	status = kstrtol(buf, 10, &value);
	if(status != 0) {
		printk("ERROR: store_trigger input\n");
		spin_unlock(&(hcsr_devps->lock_ops));
		return 4096;
	}
	
	hcsr_devps->trigger = (int) value;
	status = set_trigger_pins(hcsr_devps);
	if(status != 0) {
		printk("ERROR: Trigger Pin Not Setup\n");
		hcsr_devps->trigger = -1;
		spin_unlock(&(hcsr_devps->lock_ops));
		return 4096;
	}

	spin_unlock(&(hcsr_devps->lock_ops));
	return 4096;
}


static struct device_attribute dev_attr_trigger = {
	.attr = {
		.name = "trigger_pin",
		.mode = S_IWUSR | S_IRUGO,
	},
	.show = show_trigger,
	.store = store_trigger,
};



// ECHO =========================================================================================================

static ssize_t show_echo(struct device* dev, struct device_attribute* attr, char* buf) {
	struct hcsr_dev* hcsr_devps;
	int status;
	hcsr_devps = dev_get_drvdata(dev);

	spin_lock(&(hcsr_devps->lock_ops));
	status = snprintf(buf, 4096, "Echo IO=%d\n", hcsr_devps->echo);
	spin_unlock(&(hcsr_devps->lock_ops));

	printk("INFO: Finished snprintf\n");
	return status;
}


static ssize_t store_echo(struct device* dev, struct device_attribute* attr, const char* buf, size_t size) {
	struct hcsr_dev* hcsr_devps;
	long value;
	int status;

	hcsr_devps = dev_get_drvdata(dev);

	// LOCK
	spin_lock(&(hcsr_devps->lock_ops));

	status = kstrtol(buf, 10, &value);
	if(status != 0) {
		printk("ERROR: Store_Echo Input\n");
		spin_unlock(&(hcsr_devps->lock_ops));
		return 4096;
	}

	hcsr_devps->echo = (int) value;
	status = set_echo_pins(hcsr_devps);
	if(status != 0) {
		printk("ERROR: Echo Pin Not Set\n");
		spin_unlock(&(hcsr_devps->lock_ops));
		hcsr_devps->echo = -1;
		return 4096;
	}

	status = setup_echo_pin_interrupt(hcsr_devps);
	if(status != 0) {
		printk("ERROR: Interrupt Not Setup\n");
		spin_unlock(&(hcsr_devps->lock_ops));
		return 4096;
	}

	spin_unlock(&(hcsr_devps->lock_ops));
	return 4096;
}



static struct device_attribute dev_attr_echo = {
	.attr = {
		.name = "echo_pin",
		.mode = S_IWUSR | S_IRUGO,
	},
	.show = show_echo,
	.store = store_echo,
};




// All attributes ========================================================================

// Holds the operations that a user can use for corresponding hcsr device
static struct attribute *hcsr_attrs[] = {
	&dev_attr_echo.attr,
	&dev_attr_trigger.attr,
	&dev_attr_number_samples.attr,
	&dev_attr_sampling_period.attr,
	&dev_attr_enable_measurements.attr,
	&dev_attr_latest_distance.attr,
	NULL,

};

static const struct attribute_group hcsr_group = {
	.attrs = hcsr_attrs,
};

static const struct attribute_group *hcsr_groups[] = {
	&hcsr_group,
	NULL,
};



// ====================================================================================




/* File operations structure. Defined in linux/fs.h */
static struct file_operations hcsr_fops = {
	.owner		= THIS_MODULE,           /* Owner */
    .open		= hcsr_driver_open,        /* Open method */
    .release	= hcsr_driver_release,     /* Release method */
};



static int counter = 0;


static int create_hcsr_dev(struct P_chip *pchip) {
	struct hcsr_dev* device;
	struct device* new;
	int p;

	int verify_register;

	printk("TRY: Kmalloc\n");
	device = kmalloc(sizeof(struct hcsr_dev), GFP_KERNEL);
	if(!device) {
		printk("ERROR: kmalloc error for device\n");
		return -1;
	}
	memset(device, 0, sizeof(struct hcsr_dev));

	// Declare lock fields
	spin_lock_init(&(device->lock_ops));
	spin_lock_init(&(device->lock_irq));

	device->pchip = *pchip;
	//memcpy(&pchip_cpy, pchip, sizeof(struct P_chip));

	// Initialize misc device fields
	
	device->misc.minor = pchip->dev_no;
	device->misc.fops = &hcsr_fops;
	if(counter == 0) {
		char* the_name = "hcsr_1";
		device->misc.name = the_name;
	}
	else if(counter == 1) {
		char* new = "hcsr_2";
		device->misc.name = new;
	}
	else {
		char* new = "hcsr_2";
		device->misc.name = new;
	}
	counter += 1;
	

	// Register misc device (used for holding minor number)
	
	verify_register = misc_register(&(device->misc));
	if(verify_register != 0) {
		printk("ERROR: Misc device not registered for minor number = %d\n", pchip->dev_no);
		printk("ERROR=%d: misc register error\n", verify_register);
		return -1;
	}
	

	// Minor Number
	printk("TRY: Minor Number\n");
	device->minor = (counter+1);
	counter += 1;
	// IO Pins
	device->trigger = -1;
	device->echo = -1;

	// Sampling Parameters
	printk("TRY: Sampling Params\n");
	device->number_samples = 7;
	device->sampling_period = 200;
	device->sample_list_size = 0;

	// Create internal buffer

	for(p=0; p < device->number_samples+2; p++) {
		// Create internal sample buffer which has two additional samples for first and last sample
		struct sample_tsc* sample;
		sample = kmalloc(sizeof(struct sample_tsc), GFP_KERNEL);
		if(!sample) {
			printk("ERROR: Kmalloc error in creating internal buffer\n");
			return -1;
		} 
		memset(sample, 0, sizeof(struct sample_tsc));
		sample->order = p;
		if(p == 0) {
			// First time internal buffer is initialized
			INIT_LIST_HEAD(&(device->samples_head));

		}
		// Add to back of list
		list_add_tail(&(sample->list_ptr), &(device->samples_head));
		//pchip->device = new;
		// Update
		device->sample_list_size += 1;
	}

	// Measurement info fields
	device->enable_measurements = 0;
	device->latest_distance = 0;

	// Internal list fields
	device->sample_list_size = 0;
	device->cursor = NULL;

	// External fields
	device->external.position = 0;

	// Interrupt field
	device->irq_is_rising = true;
	device->number = MKDEV(MAJOR(dev_major), MINOR(device->misc.minor));
	printk("NUMBER=%d\n", device->number);
	// Register device in class
	printk("TRY: Register Device\n");
	//new = device_create_with_groups(&HCSR_CLASS, NULL, MKDEV(MAJOR(dev_major), MINOR(device->misc.minor)), device, hcsr_groups, "HCSR_%d", device->misc.minor);
	new = device_create_with_groups(&HCSR_CLASS, NULL, device->number, (void*) device, hcsr_groups, "HCSR_%d", device->minor);

	if(IS_ERR(new)) {
		printk("ERROR: In create device\n");
		kfree(device);
		return -1;
	}
	// Store device in global list
	list_add_tail(&(device->list_ptr), &(device_list));
	printk("SUCCESS: Registered and stored  device minor number = %d\n", pchip->dev_no);
	return 0;
}



static int devices_allocated = 0;
static int P_driver_probe(struct platform_device *dev_found)
{
	struct P_chip *pchip;
	int verify_device;
	int verify_class;
	char* my_wq_name = "my_wq";

	
	pchip = container_of(dev_found, struct P_chip, plf_dev);
	
	printk(KERN_ALERT "Found the device -- %s  %d \n", pchip->name, pchip->dev_no);
	
	if(class_enabled == false) {
		// Create class that each device is represented in sysfs
		verify_class = class_register(&HCSR_CLASS);
		if(verify_class < 0) {
			printk("ERROR: Class register\n");
			return -1;
		}
		printk("SUCCESS: New Class Created\n");
		class_enabled = true;

		// Create global driver workqueue to handle measurements
		my_wq = create_workqueue(my_wq_name);
	}
	
	printk("TRY: Creating Corresponding HCSR Device\n");
	verify_device = create_hcsr_dev(pchip);
	if(verify_device != 0) {
		printk("ERROR: probe error in creating new device\n");
		return -1;
	}
	devices_allocated += 1;
	
	return 0;
}





static int devices_removed = 0;
static int P_driver_remove(struct platform_device *pdev)
{
	
	// External
	struct hcsr_dev* tmp;
	struct hcsr_dev* cursor;
	// Internal
	struct sample_tsc* curr_tsc;
	struct sample_tsc* hold_tsc;

	if(devices_removed == devices_allocated) {
		printk("INFO: Do Nothing\n");
		return 0;
	}


	printk("TRY: Free devices\n");
	list_for_each_entry_safe(cursor, tmp, &device_list, list_ptr) {
		printk("INFO: Found Device Minor Number = %d\n", cursor->misc.minor);

		if(devices_removed == devices_allocated) {
			printk("INFO: Condition\n");
			// Unregister class once all devices cleared
			printk("TRY: Unregister Class\n");
 			class_unregister(&HCSR_CLASS);
			// Destroy workqueue
			destroy_workqueue(my_wq);
			return 0;
		}
		printk("TRY: Deallocate internal buffer\n");
		list_for_each_entry_safe(curr_tsc, hold_tsc, &(cursor->samples_head), list_ptr) {
			//printk("VAR: Order = %d\n", curr_tsc->order);
			//printk("VAR: TSC Rising = %lld\n", curr_tsc->tsc_rising);
			//printk("VAR: TSC Falling = %lld\n", curr_tsc->tsc_rising);
			list_del(&(curr_tsc->list_ptr));
			kfree(curr_tsc);
		}
		
		list_del(&(cursor->list_ptr));
		if(cursor->has_interrupt == true) {
			printk("TRY: Free IRQ\n");
			free_irq(cursor->irq_desc_echo, (void*) cursor);
		}
		//printk("TRY: Remove misc device\n");
		misc_deregister(&(cursor->misc));
		printk("TRY: Remove Device\n");
		printk("NUMBER=%d\n", cursor->number);
		//device_destroy(&HCSR_CLASS, MKDEV(dev_major, cursor->minor));
		device_destroy(&HCSR_CLASS, MKDEV(MAJOR(dev_major), MINOR(cursor->misc.minor)));
		devices_removed += 1;
		memset(cursor, 0, sizeof(struct hcsr_dev));
		kfree(cursor);
	}

	

	return 0;

};

/*
static int P_driver_probe(struct platform_device *dev_found)
{
	struct P_chip *pchip;
	int verify_device;
	int verify_class;
	char* my_wq_name = "my_wq";

	
	pchip = container_of(dev_found, struct P_chip, plf_dev);
*/
//static int devices_removed = 0;
/*
static int P_driver_remove(struct platform_device *dev_found)
{
	

	// Internal
	struct sample_tsc* curr_tsc;
	struct sample_tsc* hold_tsc;
	struct hcsr_dev* hcsr_devps;

	
	struct P_chip *pchip;
	pchip = container_of(dev_found, struct P_chip, plf_dev);
	printk("Pchip Name = %s\n", pchip->name);
	hcsr_devps = container_of(pchip, struct hcsr_dev, pchip);

	printk("DEVICE_NUMBER=%d\n", hcsr_devps->number);
	

	//printk("INFO: Found Device Minor Number = %d\n", cursor->misc.minor);
	
	printk("TRY: Deallocate internal buffer\n");
	list_for_each_entry_safe(curr_tsc, hold_tsc, &(hcsr_devps->samples_head), list_ptr) {
		//printk("VAR: Order = %d\n", curr_tsc->order);
		//printk("VAR: TSC Rising = %lld\n", curr_tsc->tsc_rising);
		//printk("VAR: TSC Falling = %lld\n", curr_tsc->tsc_rising);
		list_del(&(curr_tsc->list_ptr));
		kfree(curr_tsc);
	}
	
		
	list_del(&(hcsr_devps->list_ptr));
	if(hcsr_devps->has_interrupt == true) {
		printk("TRY: Free IRQ\n");
		free_irq(hcsr_devps->irq_desc_echo, (void*) hcsr_devps);
	}
	//printk("TRY: Remove misc device\n");
	misc_deregister(&(hcsr_devps->misc));
	printk("TRY: Remove Device\n");
	printk("NUMBER=%d\n", hcsr_devps->number);
	//device_destroy(&HCSR_CLASS, MKDEV(dev_major, cursor->minor));
	device_destroy(&HCSR_CLASS, hcsr_devps->number);
	//device_unregister(hcsr_devps);
	memset(hcsr_devps, 0, sizeof(struct hcsr_dev));
	kfree(hcsr_devps);
	
	devices_removed += 1;

	if(devices_removed == devices_allocated) {
		// Unregister class once all devices cleared
		printk("TRY: Unregister Class\n");
 		class_unregister(&HCSR_CLASS);
		// Destroy workqueue
		destroy_workqueue(my_wq);
	}

	return 0;

}
*/


static struct platform_driver P_driver = {
	.driver		= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
	.probe		= P_driver_probe,
	.remove		= P_driver_remove,
	.id_table	= P_id_table,
};

module_platform_driver(P_driver);
MODULE_LICENSE("GPL");

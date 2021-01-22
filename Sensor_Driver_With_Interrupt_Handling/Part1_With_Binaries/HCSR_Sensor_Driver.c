

#include "hcsr.h"
#include "ioctl.h"

/* ----------------------------------------------- DRIVER HCSR_Sensor_Driver --------------------------------------------------

  Driver controls <NUMBER_OF_DEVICES> Devices that eacho control a HCSR Sensor 
  	and process measurments and requests to each device

 ----------------------------------------------------------------------------------------------------------------*/



static char* user_name = "Kamal";  /* the default user name, can be replaced if a new name is attached in insmod command */


module_param(user_name,charp,0000);	//to get parameter from load.sh script to greet the user

// NUMBER_OF_DEVICES is the number of misc and hcsr devices to create (default=2)
static int NUMBER_OF_DEVICES = 2;
module_param(NUMBER_OF_DEVICES,int,0000);


// Driver workqueue for all devices that handles sampling requests
static struct workqueue_struct* my_wq;

// List holds the devices
struct list_head device_list;
LIST_HEAD(device_list);


// Names of devices
#define DEV1 "HCSR_1"
#define DEV2 "HCSR_2"
#define DEV3 "HCSR_3"
#define DEV4 "HCSR_4"
#define DEV5 "HCSR_5"
#define DEV6 "HCSR_6"


/*
* Open corresponding hcsr device for file descriptor
*/
int hcsr_driver_open(struct inode *inode, struct file *file)
{
	
	struct hcsr_dev *hcsr_devps;

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
 * Release file descriptor for corresponding hcsr device based in minor number
 */
int hcsr_driver_release(struct inode *inode, struct file *file)
{
	struct hcsr_dev* hcsr_devps;
	int minor_number = iminor(file->f_path.dentry->d_inode);
	struct hcsr_dev* tmp;
	struct hcsr_dev* cursor;
	bool found = false;
	//printk("MINOR NUMBER READ =%d\n", iminor(file->f_path.dentry->d_inode));
	
	list_for_each_entry_safe(cursor, tmp, &device_list, list_ptr) {
		// Search for matching device
		//printk("INFO: Found Device Minor Number = %d\n", cursor->misc.minor);
		if(cursor->misc.minor == minor_number) {
			hcsr_devps = cursor;
			found = true;
		}
	}

	if(found == false) {
		printk("ERROR: Could Not Find File\n");
		return -1;
	}

	if(hcsr_devps != NULL) {
		printk("SUCCESS: %s is closing\n", hcsr_devps->name);
	}
	

	return 0;
}












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

	ACTION:	Should write the start and end time stamp counters of the sensor measurement into the internal buffer
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







//________________________________________________________________________________________


void free_trigger_pins(struct hcsr_dev *hcsr_devps) {
	int io_trig = hcsr_devps->trigger;
	if(io_trig == 0) {
		gpio_free(32);
		gpio_free(11);
	}
	else if(io_trig == 1) {
		gpio_free(45);
		gpio_free(28);
		gpio_free(12);
	}
	else if(io_trig == 4) {
		gpio_free(36);
		gpio_free(6);
	}
	else if(io_trig == 5) {
		gpio_free(66);
		gpio_free(18);
		gpio_free(0);
	}
	else if(io_trig == 6) {
		gpio_free(68);
		gpio_free(20);
		gpio_free(1);
	}
	else if(io_trig == 7) {
		gpio_free(38);
	}
	else if(io_trig == 8) {
		gpio_free(40);
	}
	else {
		// Do Nothing
	}

	return;

}


/*
	Set corresponding trigger pin sequence based on user chosen IO trigger pin
*/
long set_trigger_pins(struct hcsr_dev *hcsr_devps, struct pin_params_from_user* io_params) {
	int ret;
	int io_trig = io_params->trigger;
	gpio_free(64);
	printk("VAR: IO PARAMS TRIG=%d\n", io_trig);

	if(io_trig == 0) {
		printk("TRY: Request gpio11, gpio32\n");
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
		// For triggering interrupts
		hcsr_devps->gpio_trigger = 11;
		// 33
		ret = gpio_request(32, "gpio32");
		if(ret != 0) {
			printk("ERROR: gpio32\n");
			return -1;
		}
		ret = gpio_direction_output(32, 0);
		if(ret != 0) {
			printk("ERROR: gpio32 output\n");
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
		// For triggering interrupts
		hcsr_devps->gpio_trigger = 38;
		printk("SUCCESS: gpio38\n");
	}

	else if(io_trig == 8) {
		printk("TRY: gpio40");
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
	
	return 0;
}

// _________________________________________________________________________________________	

/*
	Free matching series of echo pins based on chosen IO echo pin
*/
void free_echo_pins(struct hcsr_dev* hcsr_devps) {
	int io_echo = hcsr_devps->echo;
	
	//gpio_free(38);
	if(io_echo == 4) {
		gpio_free(36);
		gpio_free(6);

	}
	else if(io_echo == 5) {
		gpio_free(66);
		gpio_free(18);
		gpio_free(0);

	}
	else if(io_echo == 6) {
		gpio_free(68);
		gpio_free(20);
		gpio_free(1);

	}
	else if(io_echo == 9) {
		gpio_free(70);
		gpio_free(22);
		gpio_free(4);

	}
	else if(io_echo == 11) {
		gpio_free(72);
		gpio_free(44);
		gpio_free(24);
		gpio_free(5);

	}
	else if(io_echo == 13) {
		gpio_free(46);
		gpio_free(30);
		gpio_free(7);

	}
	else {
		// Do Nothing
	}
	return;
}





/*
	Set up rising edge interrupt for first echo pin in series corresponding to IO pin chosen for echo
*/
int setup_echo_pin_interrupt(struct hcsr_dev* hcsr_devps) {
	// Setup new interrupt for the gpio echo pin that is the first one for respective IO shield pin

	int verify_irq;

	printk("VAR: IRQ DESC=%d\n", hcsr_devps->irq_desc_echo);
	
	if(hcsr_devps->has_interrupt == true) {
		free_irq(hcsr_devps->irq_desc_echo, (void*) hcsr_devps);
	}

	// Request new interrupt line
	hcsr_devps->irq_desc_echo = gpio_to_irq(hcsr_devps->gpio_echo);
	if(hcsr_devps->irq_desc_echo < 0) {
		printk("ERROR=%d: gpio_to_irq\n", hcsr_devps->irq_desc_echo);
		return -1;
	}
	printk("VAR: New IRQ DESC=%d\n", hcsr_devps->irq_desc_echo);
	//printk("VAR: NAME=%s\n", hcsr_devps->name);
	// Request interrupt for gpio_echo, the interrupt is first triggered at a rising edge (gpio_trig value goes from 0 to 1)
	verify_irq = request_irq(hcsr_devps->irq_desc_echo, (irq_handler_t) irq_echo_handler, IRQF_TRIGGER_RISING, "hcsr_interrupt", (void*) hcsr_devps);
	if(verify_irq != 0) {
		printk("ERROR=%d: request_irq", verify_irq);
		return -1;
	}
	hcsr_devps->has_interrupt = true;

	printk("SUCCESS: Interrupt Requested\n");
	return 0;

}



/*
	Set up the user chosen echo pins based on IO selection
*/
long set_echo_pins(struct hcsr_dev *hcsr_devps, struct pin_params_from_user* io_params) {
	
	int ret;
	int io_echo = io_params->echo;

	
	if(io_echo == 4) {
		printk("TRY: Setting up gpio6 & gpio36\n");
		// 6
		ret = gpio_request(6, "gpio6");
		if(ret != 0) {
			gpio_free(36);
			gpio_free(6);
			ret = gpio_request(6, "gpio6");
			if(ret != 0) {
				printk("ERROR: gpio6\n");
				return -1;
			}	
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
	return 0;

}



long recreate_internal_buffer(struct hcsr_dev* hcsr_devps) {
	struct sample_tsc* cursor;
	struct sample_tsc* tmp;
	int p;

	if(hcsr_devps->currently_sampling == true) {
		printk("INFO: Currently Sampling\n");
		return 0;
	}
	//printk("VAR: DEVICE NAME MINOR =%d\n", hcsr_devps->misc.minor);
	//printk("TRY: Replace internal buffer\n");

	// LOCK: to prevent race conditions in the internal buffer and irq flag
	spin_lock_irqsave(&(hcsr_devps->lock_irq), hcsr_devps->lock_irq_flags);

	list_for_each_entry_safe(cursor, tmp, &(hcsr_devps->samples_head), list_ptr) {
		list_del(&(cursor->list_ptr));
		memset(cursor, 0, sizeof(struct sample_tsc));
		kfree(cursor);
	}

	hcsr_devps->sample_list_size = 0;
	//printk("TRY: Create New Internal Buffer\n");

	for(p=0; p < hcsr_devps->sensor_params.num_samples+2; p++) {
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
		if(p == 0) {
			//INIT_LIST_HEAD(&(hcsr_devps->samples_head));

		}
		// add to back of list
		list_add_tail(&(sample->list_ptr), &(hcsr_devps->samples_head));
		hcsr_devps->sample_list_size += 1;
	}

	printk("SUCCESS: Created Internal Buffer\n");
	// UNLOCK
	spin_unlock_irqrestore(&(hcsr_devps->lock_irq),  hcsr_devps->lock_irq_flags);
	return 0;
}




long parse_sample_input(struct sampling_params_from_user* sampling_params) {
	
	if((sampling_params->num_samples < 3) || (sampling_params->num_samples > 50)) {
		// The minimum samples is 3 since the first and last are not included in the average distance
		// Upper limit for protection from a malicious user
		printk("ERROR: sampling params number of samples must be within [3, 50]\n");
		return -1;
	}

	if((sampling_params->period < 1) || (sampling_params->period > 300)) {
		// Upper limit for protection from a malicious user
		printk("ERROR: sampling params period must be within [1, 300]\n");
		return -1;
	}

	return 0;
}



long hcsr_driver_unlocked_ioctl(struct file* file, unsigned int ioctl_num, unsigned long ioctl_param) {

	int verify_user_input;
	int verify_pins;
	int verify_interr;
	struct hcsr_dev* hcsr_devps; 
	struct pin_params_from_user io_params;
	struct sampling_params_from_user sampling_params;
	struct hcsr_dev* tmp;
	struct hcsr_dev* cursor;
	int minor_number = iminor(file->f_path.dentry->d_inode);

	bool found = false;
	list_for_each_entry_safe(cursor, tmp, &device_list, list_ptr) {
		// Search for matching device
		//printk("INFO: Found Device Minor Number = %d\n", cursor->misc.minor);
		if(cursor->misc.minor == minor_number) {
			hcsr_devps = cursor;
			found = true;
		}
	}

	if(found == false) {
		printk("ERROR: IOCTL Could Not Find File\n");
		return -1;
	}

	//printk("VAR: MINOR NUMBER = %d\n", hcsr_devps->misc.minor);

	// LOCK
	spin_lock(&(hcsr_devps->lock_ops));

	if(hcsr_devps->currently_sampling == true) {
		printk("ERROR: Write Blocked: Measurement in Progress\n");
		// UNLOCK
		spin_unlock(&(hcsr_devps->lock_ops));
		return -EINVAL;
	}

	if(ioctl_num == CONFIG_PINS) {
		// CONFIG_PINS

		//printk("INFO: CONFIG_PINS Selected\n");
		verify_user_input = copy_from_user(&io_params, (struct pin_params_from_user*) ioctl_param, sizeof(struct pin_params_from_user));
		if(verify_user_input != 0) {
			printk("ERROR=%d: copy_from_user in ioctl for CONFIG_PINS error\n", verify_user_input);
			// UNLOCK
			spin_unlock(&(hcsr_devps->lock_ops));
			return -EINVAL;
		}
		
		if(io_params.trigger == io_params.echo) {
			printk("ERROR: Pins must be different\n");
			// UNLOCK
			spin_unlock(&(hcsr_devps->lock_ops));
			return -EINVAL;
		}

		if(hcsr_devps->ioctl_pins_blocked == true) {
			// Echo and Trigger pins should only be set once for board safety reasons
			printk("ERROR: Pins should be only be set once\n");
			// UNLOCK
			spin_unlock(&(hcsr_devps->lock_ops));
			return -EINVAL;
		}
		
		//printk("VAR: IO TRIGGER = %d\n", (int) io_params.trigger);
		
		verify_pins = set_trigger_pins(hcsr_devps, &io_params);
		if(verify_pins != 0) {
			printk("ERROR: set_trigger_pins\n");
			// UNLOCK
			spin_unlock(&(hcsr_devps->lock_ops));
			return -EINVAL;
		}
		
		verify_pins = set_echo_pins(hcsr_devps, &io_params);
		if(verify_pins != 0) {
			printk("ERROR: set_echo_pins\n");
			// UNLOCK
			spin_unlock(&(hcsr_devps->lock_ops));
			return -1;
		}

		//printk("TRY: Setup Interrupt\n");

		verify_interr = setup_echo_pin_interrupt(hcsr_devps);
		if(verify_interr != 0) {
			printk("ERROR: setup echo interrupt\n");
			// UNLOCK
			spin_unlock(&(hcsr_devps->lock_ops));
			return -1;
		}

		// Pins shold not change for device once set
		hcsr_devps->ioctl_pins_blocked = true;

		// Only take measurements when trigger and echo enabled
		hcsr_devps->trigger_enabled = true;
		hcsr_devps->echo_enabled = true;
		
	}

	else if(ioctl_num == SET_PARAMETERS) {
		// SET_PARAMETERS (Sampling) (can be changed later)

		//printk("TRY: In SET_PARAMETERS\n");
		verify_user_input = copy_from_user(&sampling_params, (struct sampling_params_from_user*) ioctl_param, sizeof(struct sampling_params_from_user));
		if(verify_user_input != 0) {
			printk("ERROR=%d: copy_from_user in ioctl for SET_PARAMETERS error\n", verify_user_input);
			// UNLOCK
			spin_unlock(&(hcsr_devps->lock_ops));
			return -EINVAL;
		}
		if(parse_sample_input(&sampling_params) != 0) {
			// UNLOCK
			spin_unlock(&(hcsr_devps->lock_ops));
			return -EINVAL;
		}

		//printk("TRY: Setting sampling parameters\n");
		// Set sampling parameters
		hcsr_devps->sensor_params.num_samples     = sampling_params.num_samples;
		hcsr_devps->sensor_params.sampling_period = sampling_params.period;
		verify_user_input = recreate_internal_buffer(hcsr_devps);
		if(verify_user_input != 0) {
			printk("ERROR: Recreating internal buffer\n");
			// UNLOCK
			spin_unlock(&(hcsr_devps->lock_ops));
			return -EINVAL;
		}

	}
	else {
		printk("ERROR=%d: Unknown ioctl command\n", ioctl_num);
		// UNLOCK
		spin_unlock(&(hcsr_devps->lock_ops));
		return -EINVAL;
	}
	
	// UNLOCK
	spin_unlock(&(hcsr_devps->lock_ops));
	printk("SUCCESS: IOCTL Completed\n");
	return 0;
}




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
	// Retrieve matching device that called work_function
	hcsr_devps = container_of(work, struct hcsr_dev, my_work);

	
	while(spin_is_locked(&(hcsr_devps->lock_ops)) == 1) {
		//printk("INFO: WAIT FOR LOCK\n");
		// Wait for lock
		msleep(10);
	}

	// LOCK
	spin_lock(&(hcsr_devps->lock_ops));

	//printk("TRY: Device\n");
	//printk("INFO: Number of samples = %d\n", hcsr_devps->sensor_params.num_samples);


	hcsr_devps->take_meas = true;
	if(hcsr_devps->take_meas == true) {
		// Trigger Interrupts and take samples

		hcsr_devps->currently_sampling = true;

		printk("TRY: Work Function Take Measurements\n");
	
		// Update external buffer pointer to current position (functions like a ring buffer)
		hcsr_devps->external.position = hcsr_devps->external.position % 5;

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
			msleep((unsigned int) hcsr_devps->sensor_params.sampling_period);
		}
		printk("INFO: Sampling Finished\n");
		
		list_for_each_entry_safe(curr, temp, &(hcsr_devps->samples_head), list_ptr) {
			// Calculate distances for each internal sample, add each valid distance to average, and get last timestamp
				
			pulse_width = curr->tsc_falling - curr->tsc_rising;
			//printk("VAR: Falling-Rising INT = %d\n", (int) pulse_width);
			pulse_width = pulse_width * velocity;
			//printk("VAR: Pulse Width = %llu\n", pulse_width);
			// Have to briefly convert to int due to architecture
			curr->distance = ((int) pulse_width) / 2;

			if(curr->order > 0 && curr->order < hcsr_devps->sensor_params.num_samples+1) {
				// Add to average distance
				average_distance += ((unsigned long long) curr->distance);
				//printk("VAR: Current Average Distance = %llu\n",  average_distance);
			}
			if(curr->order == hcsr_devps->sensor_params.num_samples+1) {
				// Get last timestamp of measurement sequence
				hcsr_devps->external.buffer[hcsr_devps->external.position].tsc_last = curr->tsc_falling;
			}

		}
		
		// Briefly convert average distance due to architecture
		average_distance = (unsigned long long) ( ((int) average_distance) / hcsr_devps->sensor_params.num_samples);
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
	
	//printk("SUCCESS: Work Function Completed\n");
	// Signal sampling is completed and more samples can be taken
	hcsr_devps->currently_sampling = false;
	// UNLOCK
	spin_unlock(&(hcsr_devps->lock_ops));
	return;

}




int setup_default_echo_pins_parse(struct hcsr_dev* hcsr_devps) {
	int ret;
	int io_echo = hcsr_devps->echo;
	if(io_echo == 4) {
		printk("Setting up gpio6 & gpio36\n");
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
		hcsr_devps->gpio_echo = 6;
	}
	return 0;
}

void free_default_echo_pins_parse(struct hcsr_dev* hcsr_devps) {
	if(hcsr_devps->echo == 4) {
		gpio_free(36);
		gpio_free(6);
	}
	return;
}



int setup_default_trigger_pins_parse(struct hcsr_dev* hcsr_devps) {
	int ret;
	int io_trig = hcsr_devps->trigger;
	//printk("INFO: hcsr->trigger = %d\n", io_trig);
	if(io_trig == 7) {
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
		// 38 set dir
		ret = gpio_direction_output(38, 0);
		if(ret != 0) {
			printk("ERROR: gpio38 direction out\n");
			return -1;
		}
		hcsr_devps->gpio_trigger = 38;
		printk("SUCCESS: gpio38\n");
	}

	return 0;

}

void free_default_trigger_pins_parse(struct hcsr_dev* hcsr_devps) {
	if(hcsr_devps->echo == 7) {
		gpio_free(38);
	}
}







/*
 * Write to hcsr driver
 */
ssize_t hcsr_driver_write(struct file *file, const char *buf,
           size_t count, loff_t *ppos)
{
	struct write_argument arg;
	int verify_user_arg;
	int ret_wq;
	int i;

	struct hcsr_dev* hcsr_devps;
	bool found = false;

	int minor_number = iminor(file->f_path.dentry->d_inode);
	//printk("MINOR NUMBER READ =%d\n", iminor(file->f_path.dentry->d_inode));

	struct hcsr_dev* tmp;
	struct hcsr_dev* cursor;
	list_for_each_entry_safe(cursor, tmp, &device_list, list_ptr) {
		// Search for matching device
		//printk("INFO: Found Device Minor Number = %d\n", cursor->misc.minor);
		if(cursor->misc.minor == minor_number) {
			hcsr_devps = cursor;
			found = true;
		}
	}

	if(found == false) {
		printk("ERROR: Cound Not Find File\n");
		return -1;
	}
	
	//printk("MINOR NUMBER WRITE =%d\n", iminor(file->f_path.dentry->d_inode));
	//printk("MISC MINOR Number = %d\n", hcsr_devps->misc.minor);

	// LOCK
	spin_lock(&(hcsr_devps->lock_ops));

	if(hcsr_devps->currently_sampling == true) {
		printk("ERROR: Write Blocked: Measurement in Progress\n");
		// UNLOCK
		spin_unlock(&(hcsr_devps->lock_ops));
		return -EINVAL;
	}

	// Check for trigger and echo pins enabled status
	if(hcsr_devps->trigger_enabled == false) {
		printk("ERROR: Trigger Has To Be Enabled\n");
		// UNLOCK
		spin_unlock(&(hcsr_devps->lock_ops));
		return -EINVAL;
	}

	if(hcsr_devps->echo_enabled == false) {
		printk("ERROR: Echo Not Enabled\n");
		// UNLOCK
		spin_unlock(&(hcsr_devps->lock_ops));
		return -EINVAL;
	}


	verify_user_arg = copy_from_user(&arg, buf, sizeof(struct write_argument));
	if(verify_user_arg != 0) {
		printk("ERROR=%d: Could not copy user data\n", verify_user_arg);
		// UNLOCK
		spin_unlock(&(hcsr_devps->lock_ops));
		return -1;
	}

	if(arg.cmd < 0) {
		// Clear external buffer
		//printk("TRY: Clear external buffer\n");
		i = 0;
		for(; i < 5; i++) {
			hcsr_devps->external.buffer[i].distance = 0;
			hcsr_devps->external.buffer[i].tsc_first = 0;
			hcsr_devps->external.buffer[i].tsc_last = 0;

		}

	}
	else if(arg.cmd > 0) {
		// Take new measurements
		
		//printk("TRY: START SAMPLING\n");
		hcsr_devps->take_meas = true;
		//hcsr_devps->currently_sampling == true;
		// Initialize sampling operation and place into queue
		INIT_WORK(&(hcsr_devps->my_work), work_function);
		ret_wq = queue_work(my_wq, &(hcsr_devps->my_work));

	}
	else {
		printk("NO ACTION: cmd = 0\n");
	}

	
	while(1) {
		// Sleep until taking and processing measurements is done (worker_function should be done)
		if(hcsr_devps->currently_sampling == false) {
			//printk("INFO: Leave Loop\n");
			break;
		}
		else {
			msleep(100);
		}
	}

	// UNLOCK
	spin_unlock(&(hcsr_devps->lock_ops));
	//printk("SUCCESS: Write Completed\n");
	return 0;
}


/*
 * Read to hcsr driver
 	Return current external buffer distance and timestamp
 */
ssize_t hcsr_driver_read(struct file *file, char *buf,
           size_t count, loff_t *ppos)
{
	int ver;
	struct hcsr_dev* hcsr_devps;
	int bytes_read = 0;


	int minor_number = iminor(file->f_path.dentry->d_inode);

	struct hcsr_dev* tmp;
	struct hcsr_dev* cursor;
	bool found = false;
	list_for_each_entry_safe(cursor, tmp, &device_list, list_ptr) {
		// Search for matching device
		//printk("INFO: Found Device Minor Number = %d\n", cursor->misc.minor);
		if(cursor->misc.minor == minor_number) {
			hcsr_devps = cursor;
			found = true;
		}
	}

	if(found == false) {
		printk("ERROR: Could Not Find File\n");
		return -1;
	}
	//struct hcsr_dev *hcsr_devps = file->private_data;

	// LOCK
	spin_lock(&(hcsr_devps->lock_ops));

	if(hcsr_devps->currently_sampling == true) {
		printk("ERROR: Read Blocked: Measurement in Progress\n");
		// UNLOCK
		spin_unlock(&(hcsr_devps->lock_ops));
		return -EINVAL;
	}
	
	
	ver = copy_to_user(buf, &(hcsr_devps->external.buffer[hcsr_devps->external.position-1]), sizeof(struct fifo_buffer));
	if(ver < 0) {
		printk("ERROR=%d: Copy to user in read error\n", ver);
		// UNLOCK
		spin_unlock(&(hcsr_devps->lock_ops));
		return -1;
	}

	// Update bytes read counter
	bytes_read += sizeof(struct fifo_buffer);

	// UNLOCK
	spin_unlock(&(hcsr_devps->lock_ops));
	//Most read functions return the number of bytes put into the buffer
	return bytes_read;

}



/* File operations structure for misc devices created by module. Defined in linux/fs.h */
static struct file_operations hcsr_fops = {
    .owner		= THIS_MODULE,           /* Owner */
    .open		= hcsr_driver_open,        /* Open method */
    .release	= hcsr_driver_release,     /* Release method */
    .write		= hcsr_driver_write,       /* Write method */
    .read		= hcsr_driver_read,        /* Read method */
    .unlocked_ioctl      = hcsr_driver_unlocked_ioctl  /* Set pins or change sampling parameters */
};



/*
	Helper function for module init that creates the misc device and initializes fields
*/
int create_device(int counter) {
	//int verify_pins;
	int verify_misc;
	int p;
	struct hcsr_dev* HCSR_DEVPS;

	//printk("TRY: In create_device\n");

	HCSR_DEVPS = kmalloc(sizeof(struct hcsr_dev), GFP_KERNEL);
	if (!HCSR_DEVPS) {
		printk("ERROR: Bad Kmalloc in init\n"); return -ENOMEM;
	}
	memset(HCSR_DEVPS, 0, sizeof(struct hcsr_dev));

	// Initialize fields
	//printk("TRY: Initialize misc fields\n");
	HCSR_DEVPS->misc.minor = (counter+1);
	//printk("TRY: fops\n");
	HCSR_DEVPS->misc.fops = &hcsr_fops;
	//printk("TRY: Locks\n");
	// Set up irq and file operation locks
	spin_lock_init(&(HCSR_DEVPS->lock_irq));
	spin_lock_init(&(HCSR_DEVPS->lock_ops));
	
	
	// Name both the misc device and the hcsr_dev device the same
	if(counter == 0) {
		HCSR_DEVPS->misc.name = DEV1;
		//sprintf(HCSR_DEVPS->misc.name, "HCSR_%d", counter +1)
		//printk("TRY: hcsrdev name\n");
		HCSR_DEVPS->name = DEV1;
	}
	else if(counter == 1) {
		HCSR_DEVPS->misc.name = DEV2;
		HCSR_DEVPS->name = DEV2;

	}
	else if(counter == 2) {
		HCSR_DEVPS->misc.name = DEV3;
		HCSR_DEVPS->name = DEV3;
		
	}
	else if(counter == 3) {
		HCSR_DEVPS->misc.name = DEV4;
		HCSR_DEVPS->name = DEV4;
		
	}
	else if(counter == 4) {
		HCSR_DEVPS->misc.name = DEV5;
		HCSR_DEVPS->name = DEV5;
		
	}
	else {
		HCSR_DEVPS->misc.name = DEV6;
		HCSR_DEVPS->name = DEV6;

	}
	
	//printk("TRY: Register Device\n");
	verify_misc = misc_register(&(HCSR_DEVPS->misc));
	if(verify_misc != 0) {
		printk("ERROR=%d: did not register misc device\n", verify_misc);
		memset(HCSR_DEVPS, 0, sizeof(struct hcsr_dev));
		kfree(HCSR_DEVPS);
		return -1;
	}

	printk("SUCCESS: Allocated and Registered Device\n");

	// Interrupt should start out as rising triggered
	HCSR_DEVPS->irq_is_rising = true;
	
	// The average distance buffer pointer starts at position 0 and wraps around [0, 4]
	HCSR_DEVPS->external.position = 0;
	// Take measurements if this field is true
	HCSR_DEVPS->take_meas = false;
	// IO Pins should only be set once to prevent user from changing wires while board is running
	HCSR_DEVPS->ioctl_pins_blocked = false;
	// The general list size of the internal sampling buffer
	HCSR_DEVPS->sample_list_size = 0;

	// Default sampling parameters
	HCSR_DEVPS->sensor_params.num_samples = 5;
	HCSR_DEVPS->sensor_params.sampling_period = 200;

	// If this is true, write and read operations cannot proceed
	HCSR_DEVPS->currently_sampling = false;

	// Create internal buffer as a linked list
	//printk("TRY: Create internal buffer\n");
	for(p=0; p < HCSR_DEVPS->sensor_params.num_samples+2; p++) {
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
			INIT_LIST_HEAD(&(HCSR_DEVPS->samples_head));

		}
		// Add to back of list
		list_add_tail(&(sample->list_ptr), &(HCSR_DEVPS->samples_head));
		// Update
		HCSR_DEVPS->sample_list_size += 1;
	}

	// NOTE: T=7, I=4 are the default IO parameters
	//HCSR_DEVPS->trigger = 7;
	//HCSR_DEVPS->echo    = 4;
	// NOW: pins must be enabled usign IOCTL before write can be called
	HCSR_DEVPS->trigger = -1;
	HCSR_DEVPS->echo    = -1;
	HCSR_DEVPS->trigger_enabled = false;
	HCSR_DEVPS->echo_enabled = false;
	// The IRQ Descriptor for the base echo pin
	HCSR_DEVPS->irq_desc_echo = 0;
	// Prevents module exit from freeing an uninitialized IRQ
	HCSR_DEVPS->has_interrupt = false;

	//printk("TRY: Add Device to List");
	list_add_tail(&(HCSR_DEVPS->list_ptr), &(device_list));

	printk("SUCCESS: Device %d Registered\n", counter+1);

	return 0;

}







/*
 * Driver Initialization
 */
static bool wq_created = false;
int __init hcsr_driver_init(void)
{
	
	int verify_device;
	int counter;
	char* my_wq_name = "my_wq";

	if(wq_created == false) {
		// Create global driver workqueue for handling write calls from multiple devices
		//printk("TRY: Create Driver WorkQueue\n");
		my_wq = create_workqueue(my_wq_name);
		wq_created = true;
	}

	// Parse user input from insmod
	if(NUMBER_OF_DEVICES <= 0) {
		printk("ERROR: NUMBER_OF_DEVICES OUT OF RANGE\n");
		return -1;
	}
	if(NUMBER_OF_DEVICES > 6) {
		// Cap number of devices at 6
		NUMBER_OF_DEVICES = 6;
	}

	// Create Devices
	for(counter = 0; counter < NUMBER_OF_DEVICES; counter++) {
		// Create Devices
		printk("TRY: Create Device=%d\n", counter+1);
		verify_device = create_device(counter);
		if(verify_device != 0) {
			printk("ERROR: create_device for %d\n", counter+1);
			return -1;
		}
	}

	printk("SUCCESS: Created %d Devices\n", counter);
	
	return 0;
}


/* Driver Exit */
void __exit hcsr_driver_exit(void)
{
	

	int i;
	// devices
	struct hcsr_dev* device;
	struct hcsr_dev* tmp;
	// internal lists
	struct sample_tsc* cursor;
	struct sample_tsc* temp;

	// Destroy driver workqueue
	printk("TRY: Destroy workqueue\n");
	destroy_workqueue(my_wq);

	list_for_each_entry_safe(device, tmp, &device_list, list_ptr) {
		
		printk("TRY: Free Device Minor Number =%d\n", device->misc.minor);
		printk("TRY: Deallocate internal buffer\n");
		list_for_each_entry_safe(cursor, temp, &(device->samples_head), list_ptr) {
			printk("VAR: Order = %d\n", cursor->order);
			printk("VAR: TSC Rising = %lld\n", cursor->tsc_rising);
			//printk("VAR: TSC Falling = %lld\n", cursor->tsc_rising);
			list_del(&(cursor->list_ptr));
			memset(cursor, 0, sizeof(struct sample_tsc));
			kfree(cursor);
		}

		// View external buffer values
		for(i=0; i < 5; i++) {
			printk("VAR: External Distance = %llu\n",device->external.buffer[i].distance);
		}
	
		// Free corresponding interrupt for echo pin if an interrupt handler has been created
		if(device->has_interrupt == true) {
			printk("TRY: Free Interrupt\n");
			free_irq(device->irq_desc_echo, (void*) device);
		}

		// Free gpio pins
		printk("TRY: Free default pins\n");
		free_default_trigger_pins_parse(device);
		free_default_echo_pins_parse(device);
		printk("TRY: Free echo and trigger pins\n");
		free_trigger_pins(device);
		free_echo_pins(device);

		// Deregister device and free it
		printk("TRY: Deregister Device\n");
		misc_deregister(&(device->misc));
		memset(device, 0, sizeof(struct hcsr_dev));
		kfree(device);
	}

	printk("SUCCESS: hcsr driver removed.\n");
}



module_init(hcsr_driver_init);
module_exit(hcsr_driver_exit);
MODULE_LICENSE("GPL");


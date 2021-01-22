
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



#define DEVICE_NAME_1                 "HCSR_1"  // device name to be created and registered
#define DEVICE_NAME_2                 "HCSR_2"

#define TRIG_PIN_ON 1
#define TRIG_PIN_OFF 0

struct write_argument {
	int cmd;
};

struct fifo_buffer {
	unsigned long long distance;
	unsigned long long tsc_first;
	unsigned long long tsc_last;
};



struct return_to_user {
	struct fifo_buffer buffer[5];
};





struct sample_tsc {
	struct list_head list_ptr;
	int order;
	unsigned long long distance;
	unsigned long long tsc_rising;
	unsigned long long tsc_falling;
};

struct internal_buffer {
	int sample_counter;
	//int number_of_samples;
	struct sample_tsc tsc_samples[5];
	struct list_head buffer;
	struct sample_tsc* buff;
};





struct external_buffer {
	int position;
	bool is_ready;
	struct fifo_buffer buffer[5];
};



struct sensor_params_ {
	int num_samples;
	int sampling_period;
};



struct echo_pins_ {
	int IO;
	int pins_used;
	int pins[3];

};

struct trigger_pins_ {
	int IO;
	int pins_used;
	int first;
	int second;
	int third;
};

struct gpio_pins {
	struct echo_pins_ echo;
	struct trigger_pins_ trigger;
};





struct hcsr_dev {
	struct list_head list_ptr;
	struct miscdevice misc;
	spinlock_t lock_ops;
	spinlock_t lock_irq;
	unsigned long lock_irq_flags;
	//struct task_struct* worker_task_struct;
	struct gpio_pins pins;
	struct sensor_params_ sensor_params;
	
	struct external_buffer external;

	bool enable_write;
	bool gpio_pins_setup;
	int irq_desc_echo;
	bool irq_is_rising;
	bool take_meas;

	struct work_struct my_work;

	struct sample_tsc tsc_samples[5];
	int sample_counter;

	int sample_list_size;
	struct list_head samples_head;
	struct sample_tsc* cursor;

	int trigger;
	int echo;
	int gpio_trigger;
	int gpio_echo;

	int number_samples;
	int sampling_period;

	bool enable_measurements;
	bool currently_sampling;

	bool ioctl_pins_blocked;
	bool has_interrupt;
	bool trigger_enabled;
	bool echo_enabled;

	struct sample_tsc* curr_measurement;
	long long int latest_distance;

	//char name[20];
	char* name;
	char in_string[256];
};



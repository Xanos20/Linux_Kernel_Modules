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








#define TRIG_PIN_OFF 0
#define TRIG_PIN_ON 1




struct write_argument {
	int cmd;
};

struct gpio_pins {
	int echo;
	int trigger;
};

struct fifo_buffer {
	unsigned long long distance;
	unsigned long long tsc_first;
	unsigned long long tsc_last;
};


struct measurement_sample_params {
	int num_samples;
	int period_samples;
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

struct external_buffer {
	int position;
	bool is_ready;
	struct fifo_buffer buffer[5];
};

struct hcsr_dev {
	struct list_head list_ptr;
	struct P_chip pchip;
	struct miscdevice misc;
	int minor;
	spinlock_t lock;

	struct list_head samples_head;
	struct sample_tsc* cursor;
	int sample_list_size;

	bool currently_sampling;
	int trigger;
	int echo;
	int number_samples;
	int sampling_period;
	int enable_measurements;
	long long int latest_distance;

	bool trigger_enabled;
	bool echo_enabled;

	dev_t number;

	struct external_buffer external;

	spinlock_t lock_ops;
	spinlock_t lock_irq;
	unsigned long lock_irq_flags;

	int gpio_trigger;
	int gpio_echo;

	int irq_desc_echo;
	bool irq_is_rising;
	bool has_interrupt;

	struct work_struct my_work;
	bool take_meas;

	bool trig_set;
	bool echo_set;
	bool number_samples_set;
	bool sampling_period_set;

	bool ioctl_blocked;

	char name[20];
	char in_string[256];
};
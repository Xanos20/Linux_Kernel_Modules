
//#include "shared.h"


#include <linux/types.h>
#include <linux/spi/spi.h>

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/unistd.h>

#include <linux/netlink.h>
#include <linux/timer.h>
#include <net/genetlink.h>
#include <linux/export.h>
#include <linux/skbuff.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/timer.h>
#include <linux/export.h>
#include <net/genetlink.h>

#include "genl_ex.h"
#include "hcsr.h"

static struct timer_list timer;
static struct genl_family genl_test_family;

// Holds all sensor data
struct hcsr_dev* hcsr_devps;

// Driver workqueue for all devices that handles sampling requests
static struct workqueue_struct* my_wq;

static unsigned int animation_delay = 1000;


struct sk_buff* skb_global;
struct genl_info* info_global;

//==========================Structs=============================================

struct kernel_to_user_data {
	unsigned long long distance;
};

typedef struct {
	uint8_t led[8];
} PATTERN;

typedef struct {
	uint8_t row_cmd;
	uint8_t matrix_row;
} MATRIX;


// here first field goes first
typedef struct {
	//MATRIX off;
	MATRIX on;
} DISPLAY_TEST;

typedef struct {
	MATRIX sdm_leave;
	MATRIX r1;
} COMMANDS;



struct user_to_kernel {
    int is_ioctl;
    int is_distance_request;
    int is_display_pattern;
    int is_latest_dist;
    int trigger;
    int echo;
    int chip_select;
    PATTERN pattern;
    unsigned long long latest_dist;
    int pad1;
    int pad2;
    int pad3;
    int pad4;
    char null_byte;
};

//=================================Globals=====================================================================
struct user_to_kernel* data;
static bool is_spi_in_use = false;
static int gpio_chip_select;
static int io_chip_select;

//================================Interrupt Handling and HCSR Sensor Measurements=========================================================
















//===============================SPI Config============================================================

// SPI
#define DEVICE_NAME "device_7219"
//struct sock *netlink_socket = NULL;
struct spi_device* led_7219_device = NULL;

struct spi_message spi_msg_buf;
//static bool pins_up = false;


// Configure SPI Pin Master out slave in
static int configure_spi_clk_mosi(int i) {
	int ver;
	printk("TRY: Configure MOSI\n");
	// 24
	ver = gpio_request(24, "gpio24");
	if(ver != 0) {
		printk("ERROR=%d: gpio24\n", ver);
		return -1;
	}
	ver = gpio_direction_output(24, 0);
	if(ver != 0) {
		return -1;
	}
	// 44
	ver = gpio_request(44, "gpio44");
	if(ver != 0) {
		return -1;
	}
	ver = gpio_direction_output(44, 1);
	if(ver != 0) {
		return -1;
	}
	// 72
	ver = gpio_request(72, "gpio72");
	if(ver != 0) {
		return -1;
	}
	gpio_set_value_cansleep(72, 0);
	//_____________________________________________
	printk("TRY: SCK\n");
	// 30
	ver = gpio_request(30, "gpio30");
	if(ver != 0) {
		return -1;
	}
	ver = gpio_direction_output(30, 0);
	if(ver != 0) {
		return -1;
	}
	// 46
	ver = gpio_request(46, "gpio46");
	if(ver != 0) {
		return -1;
	}
	ver = gpio_direction_output(46, 1);
	if(ver != 0) {
		return -1;
	}

	printk("SUCCESS: MOSI and SCK Pins\n");
	return 0;
}


static void free_chip_select(int i) {
	if(io_chip_select == 0) {
		gpio_free(32);
		gpio_free(11);
	}
	else if(io_chip_select == 7) {
		gpio_free(38);
	}
	else if(io_chip_select == 8) {
		gpio_free(40);
	}
	else if(io_chip_select == 9) {
		gpio_free(70);
		gpio_free(22);
		gpio_free(4);
	}
	else {

}

}


// Configure the io gpio chip_select pin of the led device
static int configure_chip_select(int chip_select) {
	int ver;
	io_chip_select = data->chip_select;

	if(io_chip_select == 0) {
		// 11
		ver = gpio_request(11, "gpio11");
		if(ver != 0) {
			return -1;
		}
		ver = gpio_direction_output(11, 0);
		if(ver != 0) {
			return -1;
		}
		// 32
		ver = gpio_request(32, "gpio32");
		if(ver != 0) {
			return -1;
		}
		ver = gpio_direction_output(32, 0);
		if(ver != 0) {
			return -1;
		}
		gpio_chip_select = 11;

	}
	else if(io_chip_select == 7) {
		ver = gpio_request(38, "gpio38");
		if(ver != 0) {
			return -1;
		}
		ver = gpio_direction_output(38, 0);
		if(ver != 0) {
			return -1;
		}
		gpio_chip_select = 38;

	}

	else if(io_chip_select == 8) {
		printk("TRY: gpio40\n");
		ver = gpio_request(40, "gpio40");
		if(ver != 0) {
			gpio_free(40);
			ver = gpio_request(40, "gpio40");
			if(ver != 0) {
				printk("ERROR: gpio40\n");
				return -1;
			}
		}
		// turn spi device on so use value 0
		ver = gpio_direction_output(40, 0);
		if(ver != 0) {
			printk("ERROR: gpio40 direction output\n");
			return -1;
		}
		gpio_chip_select = 40;
	}

	else if(io_chip_select == 9) {
		ver = gpio_request(4, "gpio4");
		if(ver != 0) {
			return -1;
		}
		ver = gpio_request(22, "gpio22");
		if(ver != 0) {
			return -1;
		}
		ver = gpio_request(70, "gpio70");
		if(ver != 0) {
			return -1;
		}
		ver = gpio_direction_output(4, 0);
		if(ver != 0) {
			return -1;
		}
		ver = gpio_direction_output(22, 0);
		if(ver != 0) {
			return -1;
		}
		gpio_set_value_cansleep(70, 0);

		gpio_chip_select = 4;

	}
	else {

	}
	return 0;
}

//================================Send SPI Message==============================================================
//static bool t = true;
//static uint8_t intensity_value = 0x04;
struct spi_transfer spi_msg;
static bool shutdown_off = false;
static int send_msg_to_led(struct sk_buff* skb) {


	int ver;
	// Counter for specifying row register
	uint8_t row_register;
	// create spi message
	int i;
	// Data structure to hold the row register and pattern
	DISPLAY_TEST* led_sequence;

	is_spi_in_use = true;
	spi_message_init(&spi_msg_buf);


	// len is in bytes
	spi_msg.len = (unsigned) sizeof(DISPLAY_TEST);
	spi_msg.speed_hz = led_7219_device->max_speed_hz;
	

	if(shutdown_off == false) {
		// Make sure shutdown mode (which is on by default at start) is off
		for(i = 0; i < 2; i++) {
			led_sequence = (DISPLAY_TEST*) kmalloc(sizeof(DISPLAY_TEST), GFP_KERNEL);
			if(!led_sequence) {
				printk("ERROR: kmalloc\n");
				return -1;
			}
			memset(led_sequence, 0, sizeof(DISPLAY_TEST));
			led_7219_device->chip_select = 0;
			// turn shutdown mode off
			if(i == 0 && shutdown_off==false) {
				printk("INFO: First\n");
				// specify shutdown register
				led_sequence->on.row_cmd = (uint8_t) 0x0C;
				// store 1 to return to normal mode
				led_sequence->on.matrix_row = (uint8_t) 0x1;
			}
			if(i == 1) {
				printk("TRY: Turn Scan Limit On to 8\n");
				// specify set scan limit register
				led_sequence->on.row_cmd = (uint8_t) 0x0B;
				// to accept eight columns to light
				led_sequence->on.matrix_row = (uint8_t) 0x07;
			} 
			// For storing the info to send
			spi_msg.tx_buf =  led_sequence;
			//printk("TRY: Add message\n");
			spi_message_add_tail(&spi_msg, &spi_msg_buf);
			//printk("INFO: Added spi msg\n");

			// Make sure gpio chip select is off 
			ver = gpio_direction_output(gpio_chip_select, 0);
			if(ver != 0) {
				printk("ERROR: gpio_direction_output\n");
				return -1;
			}
			//printk("TRY: spi_async\n");
			// /dev/spidev1.0
			ver = spi_async(led_7219_device, &spi_msg_buf);
			if(ver != 0) {
				printk("ERROR=%d: spi_async\n", ver);
				return -1;
			}
			// Activate gpio chip select to send message
			ver = gpio_direction_output(gpio_chip_select, 1);
			if(ver != 0) {
				printk("ERROR: gpio_direction_output\n");
				return -1;
			}
		
			printk("INFO: Message sent\n");
			msleep(animation_delay);
			//printk("INFO: Clear message for new message\n");
			spi_transfer_del(&spi_msg);
			kfree(led_sequence);
		}
		shutdown_off = true;
	}
	
	// Counter for specifying the row register (should always be 1 greater than i)
	row_register = 0x01;
	printk("TRY: Send Pattern\n");
	for(i = 0; i < (8); i++) {
		// Send eight messages of the specified row and the row pattern

		led_sequence = (DISPLAY_TEST*) kmalloc(sizeof(DISPLAY_TEST), GFP_KERNEL);
		if(!led_sequence) {
			printk("ERROR: kmalloc\n");
			return -1;
		}
		memset(led_sequence, 0, sizeof(DISPLAY_TEST));

		printk("TRY: Fill in pattern for row=%d", i);
		
		// specify set scan limit register
		led_sequence->on.row_cmd = (uint8_t) row_register;
		// to accept eight columns to light
		led_sequence->on.matrix_row = (uint8_t) data->pattern.led[i];
		
		// Enables spi_async
		led_7219_device->chip_select = 0;
		// Store the pattern
		spi_msg.tx_buf =  led_sequence;
	
		printk("TRY: Add message\n");
		spi_message_add_tail(&spi_msg, &spi_msg_buf);
		printk("INFO: Added spi msg\n");
		// Make sure gpio chip select is off 
		ver = gpio_direction_output(gpio_chip_select, 0);
		if(ver != 0) {
			printk("ERROR: gpio_direction_output\n");
			is_spi_in_use = false;
			return -1;
		}
		// send message
		//printk("TRY: spi_async\n");
		// /dev/spidev1.0
		ver = spi_async(led_7219_device, &spi_msg_buf);
		if(ver != 0) {
			printk("ERROR=%d: spi_async\n", ver);
			is_spi_in_use = false;
			return -1;
		}
		// Activate gpio chip select to send message
		ver = gpio_direction_output(gpio_chip_select, 1);
		if(ver != 0) {
			printk("ERROR: gpio_direction_output\n");
			is_spi_in_use = false;
			return -1;
		}
		
		printk("INFO: Message sent\n");
		// delay to send the message
		//msleep(2000);
		msleep(100);
		//printk("INFO: Clear message for new message\n");
		spi_transfer_del(&spi_msg);
		kfree(led_sequence);
		printk("TRY: Update row_register\n");
		// Update the specified register
		row_register = row_register + 0x01;

	}


	printk("INFO: done\n");
	is_spi_in_use = false;

	return 0;
}




//===========================Sensor Config IO TRIGGER==============================================

void free_trigger_pins(int i) {
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
long set_trigger_pins(int i) {
	int ret;
	int io_trig = data->trigger;
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




//===========================Sensor Config IO ECHO & Interrupt=====================================================================

/*
	Free matching series of echo pins based on chosen IO echo pin
*/
void free_echo_pins(int i) {
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

	printk("INFO: In Interrupt\n");

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
	Set up rising edge interrupt for first echo pin in series corresponding to IO pin chosen for echo
*/
int setup_echo_pin_interrupt(int i) {
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





static void send_distance_to_user(struct sk_buff* rec_skb, struct genl_info* info)
{   
    void *hdr;
    int err;
    unsigned long long distance;
    int flags = GFP_ATOMIC;
    //char msg[GENL_TEST_ATTR_MSG_MAX];
    struct sk_buff* skb = genlmsg_new(NLMSG_DEFAULT_SIZE, flags);

    while(1) {
   		if(hcsr_devps->currently_sampling == false) {
    		break;
    	}
    	else {
    		msleep(10);
    	}
    }

    //unsigned long long distance = 255;
    distance = hcsr_devps->external.buffer[hcsr_devps->external.position-1].distance;
    printk("TRY: send_distance_to_user\n");
    printk("KVAR: Found Distance =%llu\n", distance);


    if (!skb) {
        printk(KERN_ERR "%d: ERROR: genlmsg_new", __LINE__);
        return;
    }

    hdr = genlmsg_put(skb, 0, 0, &genl_test_family, flags, GENL_TEST_C_MSG);
    if (!hdr) {
        printk(KERN_ERR "ERROR=%d: genlmsg_put !", __LINE__);
        genlmsg_cancel(skb, hdr);
        nlmsg_free(skb);
        return;
    }
    //
    err = nla_put(skb, GENL_TEST_ATTR_MSG, sizeof(unsigned long long), &(hcsr_devps->external.buffer[hcsr_devps->external.position-1].distance));
    if(err < 0) {
        printk("ERROR=%d: nla_put\n", err);
        genlmsg_cancel(skb, hdr);
        nlmsg_free(skb);
        return;
    }

    genlmsg_end(skb, hdr);
    //genlmsg_multicast(&genl_test_family, skb, 0, group, flags);
    //printk("KVAR: user portid = %d\n", info->snd_portid);
    printk("TRY: genlmsg_unicast\n");
    err = nlmsg_unicast(rec_skb->sk, skb, info->snd_portid);
    if(err < 0) {
        printk("ERROR=%d: genlmsg_unicast\n", err);
    }
    printk("INFO: Sent\n");
    return;

}






/*
	The work_queue function to activate interrupt by turning the trigger pin on and off
*/
void work_function(struct work_struct* work) {

	unsigned long long pulse_width;
	//struct hcsr_dev* hcsr_devps;
	struct sample_tsc* tmp;
	struct sample_tsc* cursor;

	struct sample_tsc* temp;
	struct sample_tsc* curr;
	unsigned long long velocity = 340; 
	unsigned long long average_distance = 0;
	//int p = 0;
	hcsr_devps->currently_sampling = true;


	/*
	while(spin_is_locked(&(hcsr_devps->lock_ops)) == 1) {
		//printk("INFO: WAIT FOR LOCK\n");
		// Wait for lock
		msleep(10);
	}
	*/

	// LOCK
	//spin_lock(&(hcsr_devps->lock_ops));


	hcsr_devps->take_meas = true;
	if(hcsr_devps->take_meas == true) {
		// Trigger Interrupts and take samples

		//hcsr_devps->currently_sampling = true;

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
	//spin_unlock(&(hcsr_devps->lock_ops));
	//send_distance_to_user(skb_global, info_global);
	return;

}










/*
	Set up the user chosen echo pins based on IO selection
*/
long set_echo_pins(int i) {
	
	int ret;
	int io_echo = data->echo;
	printk("KVAR: IO Echo = %d\n", data->echo);

	
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

	printk("TRY: setup interrupt\n");
	ret = setup_echo_pin_interrupt(1);
	if(ret < 0) {
		printk("ERROR: setup_echo_pin_interrupt\n");
		return -1;
	}
	return 0;

}



//==============================Create HCSR Data Structure===============================================================

int create_device(int i) {
	//int verify_pins;
	//int verify_misc;
	int p;

	//printk("TRY: In create_device\n");

	hcsr_devps = kmalloc(sizeof(struct hcsr_dev), GFP_KERNEL);
	if (!hcsr_devps) {
		printk("ERROR: Bad Kmalloc in init\n"); return -ENOMEM;
	}
	memset(hcsr_devps, 0, sizeof(struct hcsr_dev));

	// Initialize fields
	
	// Set up irq and file operation locks
	spin_lock_init(&(hcsr_devps->lock_irq));
	spin_lock_init(&(hcsr_devps->lock_ops));
	
	

	// Interrupt should start out as rising triggered
	hcsr_devps->irq_is_rising = true;
	
	// The average distance buffer pointer starts at position 0 and wraps around [0, 4]
	hcsr_devps->external.position = 0;
	// Take measurements if this field is true
	hcsr_devps->take_meas = false;
	// IO Pins should only be set once to prevent user from changing wires while board is running
	hcsr_devps->ioctl_pins_blocked = false;
	// The general list size of the internal sampling buffer
	hcsr_devps->sample_list_size = 0;

	// Default sampling parameters 
	hcsr_devps->sensor_params.num_samples = 3;
	hcsr_devps->sensor_params.sampling_period = 200;

	// If this is true, write and read operations cannot proceed
	hcsr_devps->currently_sampling = false;

	// Create internal buffer as a linked list
	//printk("TRY: Create internal buffer\n");
	for(p=0; p < hcsr_devps->sensor_params.num_samples+2; p++) {
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
			INIT_LIST_HEAD(&(hcsr_devps->samples_head));

		}
		// Add to back of list
		list_add_tail(&(sample->list_ptr), &(hcsr_devps->samples_head));
		// Update
		hcsr_devps->sample_list_size += 1;
	}

	// NOTE: T=7, I=4 are the default IO parameters
	//hcsr_devps->trigger = 7;
	//hcsr_devps->echo    = 4;
	hcsr_devps->trigger = -1;
	hcsr_devps->echo    = -1;

	// The IRQ Descriptor for the base echo pin
	hcsr_devps->irq_desc_echo = 0;
	// Prevents module exit from freeing an uninitialized IRQ
	hcsr_devps->has_interrupt = false;

	printk("SUCCESS: HCSR Data Structure\n");

	return 0;

}



//=============================Netlink Multigroup================================================================



//==========================Netlink Handler Messages==============================================================





//
static int config_pins(int i) {
	int ver;
	ver = configure_chip_select(8);
	if(ver != 0) {
		printk("ERROR: chip_select\n");
		return -1;
	}
	ver = configure_spi_clk_mosi(1);
	if(ver != 0) {
		printk("ERROR: configure_spi_pins\n");
		return -1;
	}
	ver = set_trigger_pins(1);
	if(ver < 0) {
		printk("ERROR: set_trigger_pins\n");
		return -1;
	}
	ver = set_echo_pins(1);
	if(ver < 0) {
		printk("ERROR: set_echo_pins\n");
		return -1;
	}

	printk("SUCCESS: config_pins\n");
	return 0;

}


/*
	Receive the message from userspace and parse it based on flags inside the user_to_kernel struct
*/
static int genl_test_rx_msg(struct sk_buff* skb, struct genl_info* info)
{
	int ver;
	int ret_wq;


    if (!info->attrs[GENL_TEST_ATTR_MSG]) {
        printk(KERN_ERR "ERROR: empty message from %d!!\n", info->snd_portid);
        printk(KERN_ERR "%p\n", info->attrs[GENL_TEST_ATTR_MSG]);
        return -EINVAL;
    }
    
   	while(1) {
   		if(hcsr_devps->currently_sampling == false) {
    		break;
    	}
    	else {
    		msleep(10);
    	}
    }

    if(info->attrs[GENL_TEST_ATTR_MSG]->nla_len == 68) {
    	// Found struct user_to_kernel
    	data = (struct user_to_kernel*) nla_data(info->attrs[GENL_TEST_ATTR_MSG]);
   	 	printk("KVAR: Trigger = %d\n", data->trigger);
    	//printk("KVAR: Data Length info = %d\n", info->attrs[GENL_TEST_ATTR_MSG]->nla_len);
    	//printk("KINFO: Sizeof struct = %d\n", sizeof(struct user_to_kernel));
    	printk("KVAR: is_distance_request=%d\n", data->is_distance_request);
    	// Should configure pins 
    	if(data->is_ioctl == 1) {
    		ver = config_pins(1);
    		if(ver < 0) {
    			printk("ERROR: config_pins\n");
    			return -1;
    		}
    	}
    	else if(data->is_ioctl == 2) {
    		free_trigger_pins(1);
    		free_echo_pins(1);
    		free_chip_select(1);
    		// mosi
			gpio_free(24);
			gpio_free(44);
			gpio_free(72);
			// sck
			gpio_free(30);
			gpio_free(46);
    		printk("KINFO: Cleared GPIO Pins\n");
    	}
    	else if(data->is_distance_request == 1) {
    		printk("KINFO: Distance Option Selected\n");
    		INIT_WORK(&(hcsr_devps->my_work), work_function);
			ret_wq = queue_work(my_wq, &(hcsr_devps->my_work));
    		//work_function(1);
    		while(1) {
    			if(hcsr_devps->currently_sampling == true) {
    				break;
    			}
    			else {
    				msleep(100);
    			}
    		}
    		while(1) {
    			if(hcsr_devps->currently_sampling == false) {
    				break;
    			}
    			else {
    				msleep(100);
    			}
    		}
    		// move into work function
    		printk("\n");
    		printk("KINFO: Send Distance To User\n");
    		send_distance_to_user(skb, info);


    	}
    	else if(data->is_display_pattern == 1) {
    		// TODO: send pattern
    		printk("KINFO: Display Pattern Selected\n");
    		while(1) {
    			if(is_spi_in_use == false) {
    				break;
    			}
    			else {
    				msleep(10);
    			}
    		}
    		ver = send_msg_to_led(skb);
    		if(ver < 0) {
    			printk("ERROR: send_msg_to_led\n");
    			return -1;
    		}
    	}
    	
    	else {
    		printk("ERROR: Unknown Flag\n");
    	}
    }
   	
   	printk("UKERNEL: Message Handled\n");
    return 0;
}




//======================Netlink Setup======================================================
static const struct genl_ops genl_test_ops[] = {
    {
        .cmd = GENL_TEST_C_MSG,
        .policy = genl_test_policy,
        .doit = genl_test_rx_msg,
        .dumpit = NULL,
    },
};

static const struct genl_multicast_group genl_test_mcgrps[] = {
    [GENL_TEST_MCGRP0] = { .name = GENL_TEST_MCGRP0_NAME, },
    [GENL_TEST_MCGRP1] = { .name = GENL_TEST_MCGRP1_NAME, },
    [GENL_TEST_MCGRP2] = { .name = GENL_TEST_MCGRP2_NAME, },
};

static struct genl_family genl_test_family = {
    .name = GENL_TEST_FAMILY_NAME,
    .version = 1,
    .maxattr = GENL_TEST_ATTR_MAX,
    .netnsok = false,
    .module = THIS_MODULE,
    .ops = genl_test_ops,
    .n_ops = ARRAY_SIZE(genl_test_ops),
    .mcgrps = genl_test_mcgrps,
    .n_mcgrps = ARRAY_SIZE(genl_test_mcgrps),
};





static int genl_test_init(void)
{
    int rc;

    printk(KERN_INFO "genl_test: initializing netlink\n");

    rc = genl_register_family(&genl_test_family);
    if (rc)
        goto failure;

    return 0;

failure:
    printk(KERN_DEBUG "genl_test: error occurred in %s\n", __func__);
    return -EINVAL;
}


static void genl_test_exit(void)
{
    del_timer(&timer);
    genl_unregister_family(&genl_test_family);
    printk("SUCCESS: genl_test_exit\n");
}


//====================SPI===========================================

static int spi_probe(struct spi_device* device_7219) {

	if(strcmp(device_7219->modalias, DEVICE_NAME) == 0 && device_7219->chip_select==1) {
		// create kernel netlink socket
		printk("INFO: Found device\n");
		// store device as global
		led_7219_device = device_7219;
		return 0;
	}
	else {
		// Unknown device
		return -1;
	}
	
}


static int spi_remove(struct spi_device* device_7219) {
	//printk("TRY: Deallocate socket\n");
	//netlink_kernel_release(netlink_socket);
	printk("TRY: Free gpio\n");
	// cs
	gpio_free(40);
	// mosi
	gpio_free(24);
	gpio_free(44);
	gpio_free(72);
	// sck
	gpio_free(30);
	gpio_free(46);
	return 0;
}



static const struct spi_device_id find_device_7219 = {
	DEVICE_NAME,
};

static struct spi_driver driver = {

	.driver = {
		.name = "7219_driver",
		.owner = THIS_MODULE,
	},
	.id_table = &find_device_7219,
	.probe  = spi_probe,
	.remove = spi_remove,
};

//====================Module Init/Exit=========================================

static int __init make_driver(void) {
	int ver;
	char* my_wq_name = "my_wq";


	// Register the SPI Led Device
	ver = spi_register_driver(&driver);
	if(ver < 0) {
		printk("ERROR=%d: spi_register_driver\n", ver);
		return -1;
	}
	printk("SUCCESS: Registered driver\n");

	// Register the Generic Netlink
	printk("TRY: genl_test_init\n");
	ver = genl_test_init();
	if(ver < 0) {
		printk("ERROR: genl_test_init\n");
		return -1;
	}
	printk("SUCCESS: Initialized Netlink\n");

	// TODO
	ver = create_device(1);
	if(ver < 0) {
		printk("ERROR: create_device\n");
		return -1;
	}
	printk("SUCCESS: HCSR Data Struct\n");
	my_wq = create_workqueue(my_wq_name);

	return 0;
}



static void __exit exit_driver(void) {
	genl_test_exit();
	printk("SUCCESS: Netlink Unregistered\n");
	spi_unregister_driver(&driver);
	printk("SUCCESS: Driver Unregistered\n");
	printk("TRY: Free echo and trigger pins\n");
	free_trigger_pins(1);
	free_echo_pins(1);
	// free chip select pin
	free_chip_select(1);
	gpio_free(40);
	destroy_workqueue(my_wq);
	memset(hcsr_devps, 0, sizeof(struct hcsr_dev));
	kfree(hcsr_devps);
	return;
}

module_init(make_driver);
module_exit(exit_driver);

//module_spi_driver(__spi_driver);
MODULE_LICENSE("GPL");


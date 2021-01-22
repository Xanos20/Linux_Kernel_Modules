#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netlink/msg.h>
#include <netlink/attr.h>
#include <pthread.h>


#include "genl_ex.h"

//static char message[GENL_TEST_ATTR_MSG_MAX];
static int send_to_kernel;
//static unsigned int mcgroups;		/* Mask of groups */
// Main will go through two iterations for default
static int dist_request_counter = 2;
// Indicates the user received a message from kernel
static int did_receive_distance = 0;
// How long reinforcements will take to reach the battlefield (changes based on measured distances)
static int animation_section_sleep_time = 3;



typedef struct {
    uint8_t led[8];
} PATTERN;

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

struct user_to_kernel* data;
struct user_to_kernel* spi_data;

// Use both distance values for comparisons
unsigned long long received_distance;
static unsigned long long last_rec_distance = 0;
// The netlink socket 
struct nl_sock* nlsock;

/*
Patterns
*/
PATTERN p1;
PATTERN p2;
PATTERN p3;
PATTERN p4;
PATTERN p5;
PATTERN p6;
PATTERN p7;
PATTERN p8;

static void make_patterns() {
	int i;
	for(i = 0; i < 8; i++) {
		p1.led[i] = 0x01;
		p2.led[i] = 0x02;
		p3.led[i] = 0x03;
		p4.led[i] = 0x04;
		p5.led[i] = 0x05;
		p6.led[i] = 0x06;
		p7.led[i] = 0x07;
		p8.led[i] = 0x08;
	}


}



static void usage(char* name)
{
	printf("Usage: %s\n"
		"	-h : this help message\n"
		"	-l : listen on one or more groups, comma separated\n"
		"	-c : the io chip select to send (default=8)\n"
		"	-d : the iterations to run (default=2)\n"
		"	-e : the io echo  to send (default=4)\n"
		"	-t : the io trigger to send (default=7)\n"
		"	-s : send to kernel (not needed)\n"
		"\n",
		name);
}




static void parse_cmd_line(int argc, char** argv)
{
	//char* opt_val;

	while (1) {
		int opt = getopt(argc, argv, "hl:m:s:c:d:e:t:");

		if (opt == EOF)
			break;

		switch (opt) {
		case 'h':
			usage(argv[0]);
			exit(EXIT_SUCCESS);
		
		case 'c':
			data->chip_select = atoi(optarg);
			break;
		case 'd':
			dist_request_counter = atoi(optarg);
			if(dist_request_counter < 2 || dist_request_counter > 50) {
				dist_request_counter = 2;
			}
			break;
		case 'e':
			data->echo = atoi(optarg);
			break;
		case 't':
			data->trigger = atoi(optarg);
		
		case 's':
			send_to_kernel = 1;
			break;

		default:
			fprintf(stderr, "Unkown option %c !!\n", opt);
			exit(EXIT_FAILURE);
		}

	}


	// Note: User input for IO pins parsed in driver
	send_to_kernel = 1;
	return;

}








static int send_msg_to_kernel(int spi_flag)
{
	struct nl_msg* msg;
	int family_id, err = 0;

	printf("UVAR: Flag=%d\n", spi_flag);

	printf("TRY: send_spi_to_kernel\n");
	family_id = genl_ctrl_resolve(nlsock, GENL_TEST_FAMILY_NAME);
	if(family_id < 0){
		fprintf(stderr, "ERROR=%d: Unable to resolve family name!\n", family_id);
		exit(EXIT_FAILURE);
	}

	msg = nlmsg_alloc();
	if (!msg) {
		fprintf(stderr, "failed to allocate netlink message\n");
		exit(EXIT_FAILURE);
	}

	if(!genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, family_id, 0, 
		NLM_F_REQUEST, GENL_TEST_C_MSG, 0)) {
		fprintf(stderr, "failed to put nl hdr!\n");
		err = -ENOMEM;
		goto out;
	}

	if(spi_flag == 2) {
		err = nla_put(msg, GENL_TEST_ATTR_MSG, sizeof(struct user_to_kernel), spi_data);
		if(err < 0) {
			fprintf(stderr, "ERROR=%d: nla_put\n", err);
			exit(-1);
		}
	}
	else if(spi_flag == 1) {
		err = nla_put(msg, GENL_TEST_ATTR_MSG, sizeof(struct user_to_kernel), data);
		if(err < 0) {
			fprintf(stderr, "ERROR=%d: nla_put\n", err);
			exit(-1);
		}
	}
	else {
		fprintf(stderr, "ERROR: Unknown Flag\n");
		exit(-1);
	}

	err = nl_send_auto(nlsock, msg);
	if (err < 0) {
		fprintf(stderr, "failed to send nl message!\n");
	}
	return 0;

out:
	nlmsg_free(msg);
	return err;
}



//=========================================================================


static int skip_seq_check(struct nl_msg *msg, void *arg)
{
	return NL_OK;
}


unsigned long long* distance;
static int print_rx_msg(struct nl_msg *msg, void* arg)
{
	struct nlattr *attr[GENL_TEST_ATTR_MAX+1];
	fprintf(stderr, "TRY: in print_rx_msg\n");

	genlmsg_parse(nlmsg_hdr(msg), 0, attr, 
			GENL_TEST_ATTR_MAX, genl_test_policy);

	if (!attr[GENL_TEST_ATTR_MSG]) {
		//fprintf(stdout, "Kernel sent empty message!!\n");
		did_receive_distance = 1;
		return NL_OK;
	}

	
	distance = (unsigned long long*) nla_data(attr[GENL_TEST_ATTR_MSG]);
	printf("UVAR: distance! = %llu\n", *distance);
	received_distance = *distance;
	did_receive_distance = 1;


	return NL_OK;
}



static void setup_netlink_socket()
{
	int family_id;
	//int grp_id;
	//unsigned int bit = 0;
	
	nlsock = nl_socket_alloc();
	if(!nlsock) {
		fprintf(stderr, "Unable to alloc nl socket!\n");
		exit(EXIT_FAILURE);
	}

	/* disable seq checks on multicast sockets */
	nl_socket_disable_seq_check(nlsock);
	nl_socket_disable_auto_ack(nlsock);

	/* connect to genl */
	if (genl_connect(nlsock)) {
		fprintf(stderr, "Unable to connect to genl!\n");
		goto exit_err;
	}

	/* resolve the generic nl family id*/
	family_id = genl_ctrl_resolve(nlsock, GENL_TEST_FAMILY_NAME);
	if(family_id < 0){
		fprintf(stderr, "Unable to resolve family name!\n");
		goto exit_err;
	}
	
    return;

exit_err:
    nl_socket_free(nlsock); // this call closes the socket as well
    exit(EXIT_FAILURE);
}



pthread_t thread_hcsr;
pthread_t thread_spi;
int main(int argc, char** argv)
{
	struct nl_cb *cb = NULL;
	int i;
	int ret;
	int ver;

	// generate spi patterns
	make_patterns();

	data = (struct user_to_kernel*) malloc(sizeof(struct user_to_kernel));
	memset(data, 0, sizeof(struct user_to_kernel));

	spi_data = malloc(sizeof(struct user_to_kernel));
	memset(spi_data, 0, sizeof(struct user_to_kernel));

	// indicate this is a config command
	data->is_ioctl = 1;
	data->is_distance_request = 0;
	data->is_latest_dist = 0;
	data->is_display_pattern = 0;
	
	// default values
	data->trigger = 7;
	data->echo = 4;
	data->chip_select = 8;
	
	for(i = 0; i < 8; i++) {
		data->pattern.led[i] = 0x00;
	}

	// retrieve optional parameters
	parse_cmd_line(argc, argv);

	// create the netlink socket
	setup_netlink_socket(&nlsock);

	// Send the IO config to driver
	ret = send_msg_to_kernel(1);
	sleep(3);

	memset(data, 0, sizeof(struct user_to_kernel));
	sleep(3);
	
	// Create callback for receiving messages from kernel
	cb = nl_cb_alloc(NL_CB_DEFAULT);
	nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, skip_seq_check, NULL);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, print_rx_msg, NULL);

	int was_last_distance_smaller = 0;
	int choice = 0;
	while(1) {
		if(choice > dist_request_counter) {
			break;
		}
		
		printf("UVAR: Choice = %d\n", choice);
		fprintf(stderr, "UVAR: Choice2=%d\n", choice%2);
		printf("\n");
		sleep(1);
		if((choice % 2) == 0) {
			// distance request
			// send distance request
			printf("TRY: Request Distance\n");
			//sleep(2);
			memset(data, 0, sizeof(struct user_to_kernel));
			data->is_distance_request = 1;
			ver = send_msg_to_kernel(1);
			if(ver < 0) {
				fprintf(stderr, "ERROR: send_to_kernel for %d iteration\n", dist_request_counter);
				exit(-1);
			}
			do {
				ret = nl_recvmsgs(nlsock, cb);
				if(did_receive_distance == 1) {
					printf("TRY: Break While Loop\n");
					break;
				}
			} while (!ret);
			// receive distance request (in global)
			// Reset flag indicating distance request was sent to user
			did_receive_distance = 0;
			printf("UINFO: RECEIVED DISTANCE!!\n");
			// Process the distance request
			if(received_distance > last_rec_distance) {
				printf("UINFO: Longer Sleep\n");
				animation_section_sleep_time += 1;
				was_last_distance_smaller = 1;
			} 
			else if(received_distance < last_rec_distance) {
				printf("UINFO: Shorter Sleep\n");
				animation_section_sleep_time -= 1;
				if(animation_section_sleep_time < 0) {
					animation_section_sleep_time = 0;
				}
				was_last_distance_smaller = 0;
				
				
			}
			else {
				// Do nothing
			}
			choice = choice +1;
		}
		else {
			// Continue animation
			memset(data, 0, sizeof(struct user_to_kernel));

			if(dist_request_counter > 0) {
				//should_stop = (should_stop * dist_request_counter);
			}
			int i;
			for(i=0; i < 8; i++) {
				// Iterate through animation
				if(i==0) {
					spi_data->pattern = p1;
				}
				else if(i==1) {
					spi_data->pattern = p2;
				}
				else if(i==2) {
					spi_data->pattern = p3;
				}
				else if(i==3) {
					spi_data->pattern = p4;
				}
				else if(i==4) {
					spi_data->pattern = p5;
				}
				else if(i==5) {
					spi_data->pattern = p6;
				}
				else if(i==6) {
					spi_data->pattern = p7;
				}
				else if(i==7) {
					spi_data->pattern = p8;
				}
				else {

				}
				spi_data->is_display_pattern = 1;
				// If check for changing intensity
				if(i == 0) {
					printf("VAR: was_last_distance_smaller=%d\n", was_last_distance_smaller);
					spi_data->pad1 = was_last_distance_smaller;
				} else {
					spi_data->pad1 = 0;
				}
				ret = send_msg_to_kernel(2);
				ret = 0;
				//printf("UVAR: Choice = %d\n", choice);
				sleep(animation_section_sleep_time);
			}

			printf("INFO: Sleep Animation Finished\n");	
			//printf("UVAR: int i = %d", i);
			//sleep(3);
			choice = choice + 1;
		} // else end

		

	}
	
	// Free echo, trigger, chip select, clock, mosi gpio pins
	memset(data, 0, sizeof(struct user_to_kernel));
	data->is_ioctl = 2;
	ret = send_msg_to_kernel(1);
	sleep(3);

	printf("UINFO: Leaving main\n");
	
	nl_cb_put(cb);
	
    nl_socket_free(nlsock);
    free(spi_data);
    free(data);
	return 0;
}

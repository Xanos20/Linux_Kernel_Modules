
#define CONFIG_PINS 0
#define SET_PARAMETERS 1


struct sampling_params_from_user {
	int num_samples;
	int period;
};

struct pin_params_from_user {
	int trigger;
	int echo;
};
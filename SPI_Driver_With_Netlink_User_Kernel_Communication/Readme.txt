
README.txt

Kamal 
Nadesan

1209035928


Instructions:
1)
In the genl_ex directory type "make"

2)
The default IO Pins Are 
- Trigger = 7
- Echo = 4
- Chip Select = 8
- To change this configuration, when you are about to run ./genl_ex ...
	./genl_ex -c <IO_chip_select> -d <distance counter> -e <IO_echo> -t <IO_trigger>

3)
Load the following modules in any order
- insmod driver_spi_nl.ko
- insmod spidev_device.ko


4)
Run ./genl_ex (with optional flags)

5)
To remove modules...
- rmmod driver_spi_nl.ko
- rmmod spidev_device.ko


Note:
ELF binaries are included in case source files were deleted in preparing to put them in the zip file

ADDITIONAL INFO
1) 
The pattern consists of a civil war type battle where soldiers line up in columns to face the enemy
2)
From the north end, the soldiers continue to move south against enemy positions
3)
Some soldier lines will fall and others will take their place
The time that it takes to replenish the ranks is dependent on sensor distance measurements (an increase in change of distance leads to an increase in time and vice versa)
4)
Eventually the north soldiers will capture around half of the battlefield

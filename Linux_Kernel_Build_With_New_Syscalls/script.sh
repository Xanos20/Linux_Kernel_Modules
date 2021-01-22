ARCH=x86i


LOCALVERSION=


INSTALL_MOD_PATH=../galileo-install


CROSS_COMPILE=i586-poky-linux-


export PATH=/opt/iot-devkit/1.7.2/sysroots/x86_64-pokysdk-linux/usr/bin/i586-poky-linux:$PATH


make -C /home/linux/Kernel_Compare/Test/kernel headers_check


make -C /home/linux/Kernel_Compare/Test/kernel headers_install


make -C /home/linux/Kernel_Compare/Test/kernel ARCH=x86 CROSS_COMPILE=i586-poky-linux-


make -C /home/linux/Kernel_Compare/Test/kernel ARCH=x86 INSTALL_MOD_PATH=/home/linux/Kernel_Compare/Test/galileo-install CROSS_COMPILE=i586-poky-linux- modules_install

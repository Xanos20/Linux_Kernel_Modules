Kamal
Nadesan

1209035928



Syscall Assignment








README


1)
Apply the patch file to the original kernel (under the .../kernel directory) 
patch -p1 < patch.txt
(command should be executed in the same directory that .../kernel is in)

2)
Place the script.sh script in the directory above /kernel
- Modify the path names to match the current system files
-  /home/linux/Kernel_Compare/Test/kernel ------> (modify)

- chmod 777 script.sh
- Now run the script ./script.sh

3)
In the galileo_install directory retrieve the bzImage
../galileo-install/lib/modules/3.19.8-yocto-standard/build/arch/x86/boot/bzImage
Then place the image on the SD card and place the SD card in the Galileo board

4)
Boot into the new kernel

5)
Compile Mydriver.c and main.c in the ../Test directory and place main and Mydriver.ko in the Galileo Board

6)
insmod Mydriver.ko

7)
Run the test program
./main show

8)
To go back to the original kernel use
- patch -R p1 < patch.txt












Examples:







[  123.550811]  [<c111d028>] ____fput+0x8/0x10
[  123.550811]  [<c104cdd1>] task_work_run+0x61/0x90
[  123.550811]  [<c1002445>] do_notify_resume+0x65/0x70
[  123.550811]  [<c145762b>] work_notifysig+0x20/0x25
[  123.550811] KTRY: dump_stack mode 0
[  123.550811] CPU: 0 PID: 309 Comm: main Tainted: G           O   3.19.8-yocto-standard #1
[  123.550811] Hardware name: Intel Corp. QUARK/GalileoGen2, BIOS 0x01000200 01/01/2014
[  123.550811]  c16488dc c16488dc cd7e3ec4 c1453bb1 cd7e3edc c1256585 c15b1144 ce60e418
[  123.550811]  ce7f95a0 cd7e3f24 cd7e3ef4 c10a27c2 ce60e420 cd7e3f24 ce7f95a0 d29c5021
[  123.550811]  cd7e3f0c c1027fa4 d29c5020 cd7e3f24 00000000 cd7a1168 cd7e3f1c c1002964
[  123.550811] Call Trace:
[  123.550811]  [<c1453bb1>] dump_stack+0x16/0x18
[  123.550811]  [<c1256585>] Pre_Handler+0x95/0xe0
[  123.550811]  [<c10a27c2>] aggr_pre_handler+0x32/0x70
[  123.550811]  [<d29c5021>] ? kbuf_driver_release+0x1/0x20 [Mydriver]
[  123.550811]  [<c1027fa4>] kprobe_int3_handler+0xb4/0x130
[  123.550811]  [<d29c5020>] ? kbuf_driver_open+0x20/0x20 [Mydriver]
[  123.550811]  [<c1002964>] do_int3+0x44/0xa0
[  123.550811]  [<c1458113>] int3+0x33/0x40
[  123.550811]  [<d29c5020>] ? kbuf_driver_open+0x20/0x20 [Mydriver]
[  123.550811]  [<d29c5021>] ? kbuf_driver_release+0x1/0x20 [Mydriver]
[  123.550811]  [<c111ceda>] ? __fput+0xaa/0x1c0
[  123.550811]  [<c111d028>] ____fput+0x8/0x10
[  123.550811]  [<c104cdd1>] task_work_run+0x61/0x90
[  123.550811]  [<c1002445>] do_notify_resume+0x65/0x70
[  123.550811]  [<c145762b>] work_notifysig+0x20/0x25
[  124.765917] 
[  124.765917] kbuf is closing
[  124.780332] KINFO: Found entry in sys_myservice_clear
UINFO: Removed d[  124.785611] KINFO: Found entry in sys_myservice_clear
umpid for open
UINFO; Removed d[  124.800250] KINFO: Found entry in sys_myservice_clear
umpid for read
[  124.806836] KINFO: Unregistered a kprobe in do_exit
[  124.810054] KINFO: Unregistered a kprobe in do_exit
UINFO: Removed dumpid for first release

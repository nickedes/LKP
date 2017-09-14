CheckLogin :

The ioctl call returns the list of handles of all logged in processes. By using the information from the login structure.

Write message :
	- Write message is modified here, 
		- If the format (sender handle, message) - This is sent to all logged in processes with timestamp smaller than message. Mapping is created for all processes.

		- If the format (sender handle, receiver handle, message) - This message is only sent to receiver handle. The handles of the sender receiver are extracted from the message and then a mapping only for that receiver process is created.

	- The mapping way of implementation helps in this part, as not much implementation is modified, only instead of creating mapping for multiple processes it is created for a single receiving process only.

Read message :
	No change

Instructions :

$ make
$ sudo insmod msyschar.ko
# If file already exists, then remove and again insmod the module
$ sudo rmmod msyschar.ko

$ dmesg|tail

# run the last command
# Example : replace 247 with the correct major number.
$ sudo mknod /dev/demo1 c 247 0

# if the major no. isn't 247, then please change the major number in file "syschar.h" on line 4.

$ gcc ioctl.c

# run processes by:

$ sudo ./a.out

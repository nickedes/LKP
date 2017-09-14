Solution Discussion :

* Device - demo1
* Login information of all processes which are logged in is stored in an array of structure.
	- Where each structure has process id of process logging in.
	- Login Timestamp
	- A unique handle provided by Process, the process can't login if the handle is not unique.
	- For Login, process uses an IOCTL Login call called IOCTL_LOGIN_PROCESS which first ensures if the handle is unique and then only stores all the above information, then the process is successfully logged in.
	- Logstore : For each login I have maintained an array that at which location(or index) the process information is stored.

* All messages written by any process are saved in an array of structure.
	- Where each structure has process id of the writing process (basically to track all information of sender - handle etc)
	- Timestamp, at which message is written
	- num_reads : This basically calculates how many no. of processes will read this message, this is calculated by condition that only those processes can read it whose timestamp is less than the message's timestamp.
	- delete field, this flag is kept for all messages so as to each the process of message deletion. When delete is 1 it means that the message is actually deleted and this space can be used by any other message.

* Write :
	- Any message is written by process in chatroom the corresponding information as stated above is stored.
	- Number of readers are updated for this message.
	- Mapping! - Now a message may be read by multiple processes and each process may read multiple messages. So a mapping (Many-to-Many kind of) is created.
		- In this mapping, the process id (or handle) of the receiving message is stored.
		- The message index (basically the index of array at which location the message is stored in buffer).
		- Flag -> this denotes that the message is read or not. If flag 0 then the message is already been read.
		- delete field, this is similar to the one created for message. So instead of deleting a mapping after a message has been read by all the processes I just invalidate that mapping and any other/new mapping can come and occupy that space.

	- So a mapping is created for each process who can read this message.

* Read :
	- When a process calls read, all the messages in the mappings in which it's id (or handle) is stored are grouped together and sent to the reading process. Message's Timestamp is also added.
	- After every read, I check if any message has been read by all its readers or not. If the num_reads field for a message reaches 0, then it has been read by all. Hence should be deleted. So in these messages and corresponding mappings the delete flag is marked 1(true).

* Logout :
	- The process's information - pid, handle, and the time at which it arrived all are deleted.
	- The entry from logstore is also removed.


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


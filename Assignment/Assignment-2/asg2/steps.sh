make;

rmmod syncdev;

insmod module/syncdev.ko;

mknod /dev/syncdev c 247 0;

echo 3 > /sys/kernel/asg2_lock;

./syncbench 8 5000000 99 1;

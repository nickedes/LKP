make;

rmmod syncdev;

insmod module/syncdev.ko;

mknod /dev/syncdev c 247 0;

python3 unit-test.py

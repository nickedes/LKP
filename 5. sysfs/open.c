#include<stdio.h>
#include<stdlib.h>
#include<sys/fcntl.h>

main()
{
   int fd = open("/dev/demo",O_RDWR);
   if(fd < 0){
       perror("open");
       exit(-1);
   }

  sleep(100);
   close(fd);
}

#include<stdio.h>
#include<stdlib.h>
#include<sys/fcntl.h>
#include<sys/ioctl.h>
#include "syschar.h"

main()
{
   int fd = open("/dev/demo2",O_RDWR);
   char read_buf[100], write_buf[100];
   char ch;
   int x;
   char handle[100], message[1000];
   unsigned login;
   if(fd < 0){
       perror("open");
       exit(-1);
   }
   printf("%s\n", "here");
   printf("Enter handle :");
   scanf("%s", handle);
   printf("%s\n", handle);
   if(ioctl(fd, IOCTL_LOGIN, handle) < 0) {
         perror("ioctl");
   }

   printf("enter operation: r- read, w -write, c - close\n");
  scanf("%c", &ch);

while(ch != 'c')
{
  switch(ch)
  {
  case 'r':
    x = read(fd, read_buf, 1000);
    printf("%s\n", read_buf);
    break;
  case 'w':
    printf("enter data: ");
    scanf("%s", write_buf);
    write(fd, write_buf, sizeof(write_buf));
    break;
  }
  printf("enter operation: r- read, w -write, c - close\n");
  scanf("%c", &ch);
}

  printf("%s\n", "here");
  if(ioctl(fd, IOCTL_LOGOUT) < 0) {
      perror("ioctl");
  }
  close(fd);
}

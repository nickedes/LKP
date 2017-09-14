#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include "syschar.h"

main()
{
    int fd = open("/dev/demo1",O_RDWR), retval;
    char read_buf[100], write_buf[100], ch, handle[100], message[1000];
    unsigned login;
    if(fd < 0){
        perror("open");
        exit(-1);
    }

    printf("Enter handle :");
    scanf("%s", handle);

    // Login ioctl call!
    retval = ioctl(fd, IOCTL_LOGIN_PROCESS, handle);
    if(retval == -1)
    {
        printf("Entered Handle is not unique.Try again\n");
        close(fd);
        return;
    }
    else if(retval < 0)
    {
        perror("ioctl");
    }

    printf("enter operation: r- read, w -write, c - close\n");
    scanf("%c", &ch);

    do
    {
        switch(ch)
        {
            case 'r':
                retval = read(fd, read_buf, 1000);
                // print the read material
                printf("%s\n", read_buf);
                break;
            case 'w':
                printf("enter data :");
                scanf("%s", write_buf);
                write(fd, write_buf, sizeof(write_buf));
                break;
            default:
                // printf("Operation not supported!\n");
                break;
        }
        printf("enter operation: r- read, w -write, c - close\n");
        scanf("%c", &ch);
    }while(ch != 'c');

    if(ioctl(fd, IOCTL_LOGOUT_PROCESS) < 0) {
        perror("ioctl");
    }
    close(fd);
}

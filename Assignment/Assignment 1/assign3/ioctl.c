#include<stdio.h>
#include<stdlib.h>
#include<sys/fcntl.h>
#include<sys/ioctl.h>
#include "syschar.h"
#include <string.h>
struct info
{
char sender[100];
char receiver[100];
char message[100];
};

static struct token
{
char handle[100];
int key;
}*pair;

main()
{
    int fd = open("/dev/demo3",O_RDWR), retval;
    char read_buf[100], *write_buf, data[100];
    char ch;
    int x, key, i, ctr=0;
    char handle[100], message[1000], receiver[100];
    unsigned login;

    if(fd < 0){
        perror("open");
        exit(-1);
    }

    pair = malloc(sizeof(*pair));
    printf("Enter handle :");
    scanf("%s", handle);
    printf("Enter Key (Provided by CA) :");
    scanf("%d", &key);

    strcpy(pair->handle, handle);
    pair->key = key;
    // login ioctl call!
    retval = ioctl(fd, IOCTL_LOGIN, (char *)pair);

    if(retval == -1)
    {
      printf("Invalid Key, you cannot login\n");
      close(fd);
      return ;
    }
    else if(retval < 0)
    {
      perror("ioctl");
    }

    printf("enter operation: r- read, w -write, l - check login,c - close\n");
    scanf("%c", &ch);

    struct info *m = malloc(sizeof(*m));

    while(ch != 'c')
    {
        switch(ch)
        {
        case 'r':
            x = read(fd, read_buf, 1000);
            printf("%s\n", read_buf);
            break;
        case 'w':
            printf("Enter message : ");
            scanf("%s", data);
            printf("%s\n", data);

            for(i = 0; i < strlen(data); i++)
              if(data[i] == ',')
                ctr++;

            char* token = strtok(data, ",");
            strcpy(m->sender,token);
            if(ctr > 1)
            {
              token = strtok(NULL, ",");
              strcpy(m->receiver,token);
              token = strtok(NULL, ",");
              strcpy(m->message,token);
            }
            else
            {
              token = strtok(NULL, ",");
              strcpy(m->message,token);
            }
            write_buf = (char *)m;
            write(fd, write_buf, sizeof(write_buf));
            break;
        case 'l':
            x = ioctl(fd, CHECK_LOGIN, message);
            printf("%s\n", message);
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

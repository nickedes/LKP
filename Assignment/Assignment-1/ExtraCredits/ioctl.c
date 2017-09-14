#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
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
    int fd = open("/dev/demo3",O_RDWR), retval, x, ch, key, i, ctr=0;
    char read_buf[100], *write_buf, data[100], handle[100], message[1000];
    unsigned login;

    if(fd < 0){
        perror("open");
        exit(-1);
    }

    // for creating a handle-key pair for user process
    pair = malloc(sizeof(*pair));
    printf("Enter handle :");
    scanf("%s", handle);
    printf("Enter Key (Provided by CA) :");
    scanf("%d", &key);

    strcpy(pair->handle, handle);
    pair->key = key;

    // login ioctl call!
    retval = ioctl(fd, IOCTL_LOGIN_PROCESS, (char *)pair);
    if(retval == -1)
    {
        // login allowed only when key is valid
        printf("Invalid Key, you cannot login\n");
        close(fd);
        return ;
    }
    else if(retval == 1)
    {
        printf("Entered Handle is not unique.Try again\n");
        close(fd);
        return;
    }
    else if(retval < 0)
    {
        perror("ioctl");
    }


    // info - input format for write!
    // malloc space for input message struct
    struct info *m = malloc(sizeof(*m));

    printf("Input your choice: 1 - read, 2 -write, 3 - check login,4 - close\n");
    scanf("%d", &ch);

    while(ch != 4)
    {
        switch(ch)
        {
        case 1:
            x = read(fd, read_buf, 1000);
            printf("%s\n", read_buf);
            break;
        case 2:
            printf("Enter message : ");
            scanf("%s", data);

            for(i = 0; i < strlen(data); i++)
              if(data[i] == ',')
                ctr++;

            // Extract the handles and message
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
        case 3:
            // get all logged in handles
            x = ioctl(fd, CHECK_LOGIN, message);
            printf("%s\n", message);
            break;
        }
        printf("Input your choice: 1 - read, 2 -write, 3 - check login,4 - close\n");
        scanf("%d", &ch);
    }

    if(ioctl(fd, IOCTL_LOGOUT_PROCESS) < 0) {
      perror("ioctl");
    }
    close(fd);
}
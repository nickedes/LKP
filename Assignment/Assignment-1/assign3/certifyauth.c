#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include "syschar.h"
#include <string.h>
#include <time.h>

// maintain all handles and generated keys
static struct token
{
    char handle[100];
    int key;
}tokens[100];

static int token_index = 0;

// a random key generation function
int keygen()
{
    srand(time(NULL));
    return rand();
}

main()
{
    int fd = open("/dev/demo3",O_RDWR), i, flag;
    char handle[100], *usertoken, ch;
    struct token *pair;
    if(fd < 0){
        perror("open");
        exit(-1);
    }
    // My Roll no. is used for verifying Certificate authority
    if(ioctl(fd, LOGIN_CA, 17111056) < 0) {
        perror("ioctl");
    }

    printf("enter operation: g - Get Key, c - close\n");
    scanf("%c", &ch);

    while(ch != 'c')
    {
        switch(ch)
        {
            case 'g':
                printf("Enter Handle of process: ");
                scanf("%s", handle);
                flag = 0;
                for(i = 0; i < token_index; i++)
                {
                    if(strcmp(tokens[i].handle, handle) == 0)
                    {
                        printf("Key generated is : %d. Use this while logging in chatroom.\n", tokens[i].key);
                        flag = 1;
                        break;
                    }
                }

                if(flag)
                    break;
                pair = malloc(sizeof(*pair));
                strcpy(pair->handle, handle);
                strcpy(tokens[token_index].handle, handle);
                pair->key = keygen();
                tokens[token_index].key = pair->key;
                token_index++;
                usertoken = (char *)pair;
                if(ioctl(fd, IOCTL_SETPAIR, usertoken) < 0)
                {
                    perror("ioctl");
                }
                printf("Key generated is : %d. Use this while logging in chatroom.\n", pair->key);
                break;

        }
        printf("enter operation: g - Get Key, c - close\n");
        scanf("%c", &ch);
    }
    
    close(fd);
}

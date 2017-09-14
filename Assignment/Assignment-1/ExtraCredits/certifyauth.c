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
    int fd = open("/dev/demo3",O_RDWR), i, flag, ch;
    char handle[100], *usertoken;
    struct token *pair;
    if(fd < 0){
        perror("open");
        exit(-1);
    }
    // My Roll no. is used for verifying Certificate authority
    if(ioctl(fd, LOGIN_CA, 17111056) < 0) {
        perror("ioctl");
    }

    printf("Enter operation: 1 - Get Key, 2 - close\n");
    scanf("%d", &ch);

    while(ch != 2)
    {
        switch(ch)
        {
            case 1:
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
                printf("Key generated is : %d . Use this while logging in chatroom.\n", pair->key);
                break;
            default:
                break;
        }
        printf("Enter operation: 1 - Get Key, 2 - close\n");
        scanf("%d", &ch);
    }
    
    close(fd);
}

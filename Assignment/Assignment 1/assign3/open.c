#include<stdio.h>
#include<stdlib.h>

static struct login{
    int pid;
    int time;
    char *handle;
}logins[1000];

static int login_index = 0;
static int logstore[10000] = {0};

main()
{
  int sender = 124;
  char *h;
   h = "opll";
   logins[login_index].pid = 124;
   logins[login_index].time = 1;
   logins[login_index].handle = h;
   logstore[124]=login_index;
   login_index++;

   printf("handle %s\n", logins[ logstore[sender] ].handle);
   return 0;
}

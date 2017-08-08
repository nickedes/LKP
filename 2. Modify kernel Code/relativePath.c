/*
	Modify in your linux kernel source :
	In file fs/open.c, add the main code in do_sys_open() on line 1040.
	Task : Print relative path for all the files which have my home directory in the absolute path.
*/
#include <stdio.h>
#include <string.h>

int main()
{	
	int i, j = 0;
	char *parentDir = "/home/nickedes/";
	// filename will be as a param in "fs/open.c" line 1040 in do_sys_open()
	char *filename = "/random/path"
	char *x = filename;
	char temp[1000];
	if (strstr(x,parentDir) != NULL )
	{
		for (i = strlen(parentDir); x[i] != '\0'; ++i)
		{
			temp[j++] = x[i];
		}
		temp[j] = '\0';
		printk("%s", temp);
	}
	else
	{
		printk("%s", filename);
	}
}
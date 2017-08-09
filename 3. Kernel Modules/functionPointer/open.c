/*
	These are the modifications made in "fs/open.c" file code around do_sys_open() function
*/

void (* fn) (char *p) = NULL;	/* Function pointer */
EXPORT_SYMBOL(fn);	/* Export the fp */

long do_sys_open(int dfd, const char __user *filename, int flags, umode_t mode)
{

	if(fn != NULL)
	{
		fn(filename);
	}
}
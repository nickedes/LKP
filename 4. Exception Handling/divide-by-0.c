#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void handler(int signal)
{
	if(signal == SIGFPE)
	{
		printf("%s\n", "caught it");
	}
}

int main()
{	
	if(signal(SIGFPE, handler) == SIG_ERR)
	{
		printf("%s\n", "not handled");
	}
	int i = 3, j = 3;

	j = i / (i-j);

	printf("%d\n", j);
	return 0;
}
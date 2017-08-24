#include<stdio.h>
#include<stdlib.h>
#include<sys/fcntl.h>

main()
{
	 int fd = open("/dev/demo2",O_RDWR);
	 char *read_buf, *write_buf;
	 char ch;
	 int x;
	 if(fd < 0){
			 perror("open");
			 exit(-1);
	 }

	printf("enter operation: r- read, w -write\n");
	scanf("%c", &ch);

	switch(ch)
	{
	case 'r':
		x = read(fd, read_buf, 6);
		printf("%d", x);
		break;
	case 'w':
		printf("enter data: ");
		scanf(" %[^\n]", write_buf);
		write(fd, write_buf, sizeof(write_buf));
		break;
	}
	close(fd);
}

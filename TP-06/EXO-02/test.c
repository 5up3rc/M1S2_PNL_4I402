#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAGIC 'N'
#define HELLO _IOR(MAGIC, 0, char *)
#define WHO _IOR(MAGIC, 1, char *)

int main(int argc, char **argv)
{
	int file = open("/dev/hello", O_RDONLY);
	char query[1024];
	char answer[1024];

	printf("ASK what you want : ");
	scanf("%s", query);
	ioctl(file, WHO, (unsigned long)query);
	ioctl(file, HELLO, (unsigned long)answer);

	printf("\nANSWER : %s\n", answer);

	return EXIT_SUCCESS;
}

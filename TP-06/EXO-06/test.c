#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define MAGIC 'N'
#define GET_SAMPLE _IOR(MAGIC, 0, char *)
#define TASKMON_STOP _IOR(MAGIC, 1, void *)
#define TASKMON_START _IOR(MAGIC, 2, void *)
#define TASKMON_SET_PID _IOW(MAGIC, 3, int)

int main(int argc, char **argv)
{
	int file = open("/dev/taskmonitor", O_RDONLY);
	char query[1024];
	char answer[1024];
	int pid = 0;

	while (1) {
		printf("ASK what you want (get|stop|start|set|quit) : ");
		scanf("%s", query);

		if (strncmp(query, "get", 3) == 0) {
			ioctl(file, GET_SAMPLE, (unsigned long)answer);
			printf("\nANSWER : %s\n", answer);
		} else if (strncmp(query, "stop", 4) == 0) {
			ioctl(file, TASKMON_STOP, NULL);
		} else if (strncmp(query, "start", 5) == 0) {
			ioctl(file, TASKMON_START, NULL);
		} else if (strncmp(query, "set", 3) == 0) {
			printf("\nPID : ");
			scanf("%d", &pid);
			ioctl(file, TASKMON_SET_PID, pid);
		} else if (strncmp(query, "quit", 4) == 0) {
			break;
		}
	}

	return EXIT_SUCCESS;
}

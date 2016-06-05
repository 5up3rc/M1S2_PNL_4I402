#ifndef PROJET_H
#define PROJET_H

static int major;
static const char *name = "projet";

typedef struct {
	int pid;
	int sig;
} pidsig; /*for kill*/

typedef struct {
	int *pid
	char *buf;
} pidbuf; /*for wait*/

typedef struct {
	char *name;
	char *buf;
} namebuf; /*for modinfo*/

#define MAGIC 'N'
#define LIST _IOR(MAGIC, 0, char *)
#define KILL _IOR(MAGIC, 1, pidsig *)
#define WAIT _IOR(MAGIC, 2, pidbuf *)
#define MEMINFO _IOR(MAGIC, 3, struct sysinfo)
#define MODINFO _IOR(MAGIC, 4, namebuf *)

#endif

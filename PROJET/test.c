#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <argp.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/sysinfo.h>

#include "projet.h"

void displaySysinfo(struct sysinfo mysi)
{
	printf("Seconds since boot ------------------- %li\n", mysi.uptime);
	printf("Total usable main memory size -------- %8lu\n", mysi.totalram);
	printf("Available memory size ---------------- %8lu\n", mysi.freeram);
	printf("Amount of shared memory	-------------- %8lu\n", mysi.sharedram);
	printf("Memory used by buffers --------------- %8lu\n", mysi.bufferram);
	printf("Total swap space size ---------------- %8lu\n", mysi.totalswap);
	printf("Swap space still available ----------- %8lu\n", mysi.freeswap);
	printf("Number of current processes ---------- %hu\n", mysi.procs);
	printf("Total high memory size --------------- %8lu\n", mysi.totalhigh);
	printf("Available high memory size ----------- %8lu\n", mysi.freehigh);
	printf("Memory unit size in bytes ------------ %8i\n",  mysi.mem_unit);
}

const char *argp_program_version = "ProjetPNL";
const char *argp_program_bug_address = "<somewhere@nothere>";

/* Program documentation. */
static char doc[] = "Documentation du Projet PNL";

/* A description of the arguments we accept. */
static char args_doc[] = "ARG1 ARG2 [ARGS...]";

/* The options we understand. */
static struct argp_option options[] = {
	{"list",	'l', 0, 0, "List of executed commands"},
	{"kill",	'k', 0, 0, "Kill process"},
	{"wait",	'w', 0, 0, "Wait processes"},
	{"meminfo",	'm', 0, 0, "Memory info"},
	{"modinfo",	'M', 0, 0, "Module info"},
	{0}
};

/* Used by main to communicate with parse_opt. */
struct arguments {
	char *args[2];                /* arg1 & arg2 */
	char* pid;
	char* sig;
	int pids[10];
	char name[64];
};

/* Parse a single option. */
static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	/* Get the input argument from argp_parse, which we
	know is a pointer to our arguments structure. */
	struct arguments *arguments = state->input;
	
	int file = open("/dev/ProjetPNL", O_RDONLY);
	int i;
	char list[256];
	struct sysinfo mysi;
	struct pidsig ps;
	struct pidbuf pb;
	struct namebuf nb;

	switch (key) {
	case 'l':
		if (state->arg_num > 0) {
			argp_usage (state);
			break;
		}
		ioctl(file, LIST, &list);
		printf("%s\n", list);
		break;
	case 'k':
		if (state->arg_num != 2) {
			argp_usage (state);
			break;
		}
		ps.pid = atoi(state->argv[state->next]);
		ps.sig = atoi(state->argv[(state->next)+1]);
		ioctl(file, KILL, &ps);
		break;
	case 'w':
		if (state->arg_num < 1) {
			argp_usage (state);
			break;
		}
		for(i=0; i<argc-1; i++){
			pb->pids[i] = atoi(state->argv[state->next]+i);
		}
		ioctl(file, WAIT, &pb);
		break;
	case 'm':
		if (state->arg_num > 0) {
			argp_usage (state);
			break;
		}
		ioctl(file, MEMINFO, &mysi);
		displaySysinfo(mysi);
		break;
	case 'M':
		if (state->arg_num != 1) {
			argp_usage (state);
			break;
		}
		snprintf(nb->name, sizeof(state->argv[state->next]), "%s", 
						state->argv[state->next]);
		ioctl(file, MODINFO, &nb);
		printf("%s\n", modinfo);
		break;
		
	case ARGP_KEY_ARG:
		if (state->arg_num > 10)
			/* Too many arguments. */
			argp_usage (state);
		arguments->args[state->arg_num] = arg;
		break;

	case ARGP_KEY_END:
		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}
	
	return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };


int main(int argc, char **argv)
{
	int i;
	struct arguments arguments;

	/* Default values. */
	arguments.pid = "";
	arguments.sig = "";
	for (i = 0; i < 10 ; i++)
		arguments.pids[i] = 0;

	/* Parse our arguments; every option seen by parse_opt will
	be reflected in arguments. */
	argp_parse (&argp, argc, argv, 0, 0, &arguments);
	
	return EXIT_SUCCESS;
}

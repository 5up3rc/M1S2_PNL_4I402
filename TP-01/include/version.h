#ifndef VERSION_H
#define VERSION_H

/*struct version {
	unsigned short major;
	unsigned long minor;
	char flags;
};*/

struct version {
	unsigned short major;
	char flags;
	unsigned long minor;
};

int is_unstable(struct version *v);

int is_unstable_bis(struct version *v);

void display_version(struct version *v, int(*pointeur_fonction)(struct version *));

int cmp_version(struct version *v, int major, int minor);

#endif

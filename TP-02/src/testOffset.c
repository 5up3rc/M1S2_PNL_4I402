#include<stdlib.h>
#include<stdio.h>

#include "../include/commit.h"
#include "../include/version.h"


struct commit *commit_of(struct version *version)
{
	unsigned long offset;	
	struct commit *com = (struct commit*) malloc (sizeof(struct commit));
	offset = (unsigned long) &(com->version) - (unsigned long) com;
	free (com);
	printf ("offset = %ld, addr = %ld\n", offset, (unsigned long)version);
	return (struct commit *) ((unsigned long)version - offset);
}

int main(int argc, char const* argv[])
{	
	
	struct commit *c = (struct commit*)malloc(sizeof(struct commit));
	c->id = 14;
	c->version.major = 3;
	c->version.minor = 5;
	c->version.flags = 0;
	c->comment = "commentaire hello world!\n";
	c->next = NULL;
	c->prev = NULL;

	printf("id = %p \nversion.major = %p \nversion.minor = %p \nversion.flags = %p \ncomment = %p \nnext = %p \nprev = %p\n", &(c->id), &(c->version.major), &(c->version.minor), &(c->version.flags), &(c->comment), &(c->next), &(c->prev));
	/*printf("ad1  = %p, ad2 = %p\n", c, (commit_of(&(c->version))));*/
	return 0;
	
}

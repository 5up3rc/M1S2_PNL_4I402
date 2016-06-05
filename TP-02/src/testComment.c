#include<stdlib.h>
#include<stdio.h>

#include"../include/comment.h"

int main(int argc, char const *argv[])
{
	struct comment *c;
	if(c = new_comment(6, "test1", 10, "J. Sopena", 9, "Un texte"))
		display_comment(c);
	printf("\n");
	
	if(c = new_comment(6, "test2", -1, "J. Sopena", 9, "Un texte"))
		display_comment(c);
	printf("\n");

	if(c = new_comment(6, "test2", 10, "J. Sopena", -1, "Un texte"))
		display_comment(c);
	printf("\n");

	return 0;
}

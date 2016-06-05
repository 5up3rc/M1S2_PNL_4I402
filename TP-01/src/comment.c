#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#include"../include/comment.h"

struct comment *new_comment(
	int title_size, char *title,
	int author_size, char *author,
	int text_size, char *text)
{
	struct comment *c = (struct comment *) malloc(sizeof(struct comment));

	c->title_size = title_size;
	if(! (c->title = malloc(title_size)))
		return NULL;
	memcpy(c->title, title, title_size);

	c->author_size = author_size;
	if(! (c->author = malloc(author_size)))
		return NULL;
	memcpy(c->author, author, author_size);

	c->text_size = text_size;
	if(! (c->text = malloc(text_size)))
		return NULL;
	memcpy(c->text, text, text_size);

	return c;
}

void display_comment(struct comment *c)
{
	printf("%s from %s \"%s\"\n", c->title, c->author, c->text);
}


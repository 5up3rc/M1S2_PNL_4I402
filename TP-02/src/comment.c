#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#include"../include/comment.h"

struct comment *new_comment(
	int title_size, char *title,
	int author_size, char *author,
	int text_size, char *text)
{
	int i;
	struct comment *c = (struct comment *) malloc(sizeof(struct comment));


	for(i=0; title[i]!='\0'; ++i);
	if(title_size < i) title_size = i;
	
	c->title_size = title_size;
	if(! (c->title = malloc(title_size)))
		return NULL;
	memcpy(c->title, title, title_size);
	
	
	for(i=0; author[i]!='\0'; ++i);
	if(author_size < i) author_size = i;
	
	c->author_size = author_size;
	if(! (c->author = malloc(author_size)))
		return NULL;
	memcpy(c->author, author, author_size);


	for(i=0; text[i]!='\0'; ++i);
	if(text_size < i) text_size = i;
	
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


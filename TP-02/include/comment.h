#ifndef COMMENT_H
#define COMMENT_H

struct comment {
	int title_size;
	char *title;
	int author_size;
	char *author;
	int text_size;
	char *text;
};

struct comment *new_comment(
	int title_size, char *title,
	int author_size, char *author,
	int text_size, char *text);

void display_comment(struct comment *v);

#endif

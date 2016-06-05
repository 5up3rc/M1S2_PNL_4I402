#include<stdlib.h>
#include<stdio.h>

#include "../include/commit.h"

int main(int argc, char const* argv[])
{
	struct history *first = new_history("First !");
	struct commit *tmp, *victim, *last;
	

	display_commit(first->commit_list);
	printf("\n"); 
printf("step 1 done\n");

	display_history(first);
printf("step 2 done\n");
	tmp = add_minor_commit(first->commit_list, "Work 1");
	tmp = add_minor_commit(tmp, "Work 2");
	victim = add_minor_commit(tmp, "Work 3");
	last = add_minor_commit(victim, "Work 4");
	display_history(first);
printf("step 3 done\n");
	del_commit(victim);
	display_history(first);
printf("step 4 done\n");
	tmp = add_major_commit(last, "Realse 1");
	tmp = add_minor_commit(tmp, "Work 1");
	tmp = add_minor_commit(tmp, "Work 2");
	tmp = add_major_commit(tmp, "Realse 2");
	tmp = add_minor_commit(tmp, "Work 1");
	display_history(first);
printf("step 5 done\n");
	add_minor_commit(last, "Oversight !!!");
	display_history(first);
printf("step 6 done\n");
	infos(first->commit_list, 1, 2);

	infos(first->commit_list, 1, 7);

	infos(first->commit_list, 4, 2);

	/*freeHistory(first->commit_list);
	display_history(first);
	free(first->commit_list);
	free(first);
	free(victim->comment);
	free(victim);*/

	return 0;
}

#include<stdlib.h>
#include<stdio.h>
#include <string.h>


#include "../include/commit.h"
#include "../include/version.h"

#define size_com 255
static int nextId = 0;

struct history *new_history(char *name)
{
	struct history *h = (struct history*)malloc(sizeof(struct history));
	h->commit_count = 0;
	h->name = (char*)malloc(sizeof(char)*14);
	strcpy(h->name, name);
	h->commit_list = new_commit(0,0,"fantome");

	return h; 	
}


struct commit *last_commit(struct history *hist)
{
	return list_entry(hist->commit_list->list.prev, struct commit, list);
}


/**
  * new_commit - alloue et initialise une structure commit correspondant au parametre
  * @major:      numero de version majeure
  * @minor:      numero de version mineure
  * @comment:    pointeur vers une chaine de caracteres contenant un commentaire
  */
struct commit *new_commit(unsigned short major, unsigned long minor, char *comment)
{
	struct commit *c = (struct commit*)malloc(sizeof(struct commit));
	c->id = nextId++;
	c->version.major = major;
	c->version.minor = minor;
	c->version.flags = 0;
	c->comment = (char*)malloc(sizeof(char)*size_com);
	strcpy(c->comment, comment);
	INIT_LIST_HEAD(&(c->list));	
	INIT_LIST_HEAD(&(c->major_list));	
	c->major_parent = c;
	
	c->ops = (struct commit_ops*)malloc(sizeof(struct commit_ops));
	if(minor == 0) {
		c->ops->display = display_major_commit;
		c->ops->extract = extract_major;
	} else {
		c->ops->display = display_commit;
		c->ops->extract = extract_minor;
	}
	
	return c;
}

/**
  * insert_commit - insert sans le modifier un commit dans la liste doublement chainee
  * @from:       commit qui deviendra le predecesseur du commit insere
  * @new:        commit a inserer - seul ses champs next et prev seront modifie
  */
static struct commit *insert_commit(struct commit *from, struct commit *new)
{
	list_add(&(new->list), &(from->list));
	return new;
}

/**
  * add_minor_commit - genere et insert un commit correspondant a une version mineure
  * @from:           commit qui deviendra le predecesseur du commit insere
  * @comment:        commentaire du commit
  */
struct commit *add_minor_commit(struct commit *from, char *comment)
{
	struct commit* c = new_commit(from->version.major, from->version.minor+1, comment);
	c->major_parent = from->major_parent;
	return insert_commit(from, c);
}

/**
  * add_major_commit - genere et insert un commit correspondant a une version majeure
  * @from:           commit qui deviendra le predecesseur du commit insere
  * @comment:        commentaire du commit
  */
struct commit *add_major_commit(struct commit *from, char *comment)
{
	struct commit* c = new_commit(from->version.major+1, 0, comment);
	if(from->version.minor != 0) {
		list_add(&(c->major_list), &(from->major_parent->major_list));
		return insert_commit(from, c);
	}
	list_add(&(c->major_list), &(from->major_list));
	return insert_commit(from, c);
}

/**
  * del_commit - extrait le commit de l'historique
  * @victim:         commit qui sera sorti de la liste doublement chainee
  */
struct commit *del_commit(struct commit *victim)
{
	return (*victim->ops->extract)(victim);
}	


struct commit *extract_major(struct commit *victim)
{
	struct commit *c, *p;
	list_for_each_entry_safe(c, p, &(victim->list), list) {
		if(c->major_parent->id == victim->id) del_commit(c);
	}
	
	list_del_init(&(victim->list));
	list_del_init(&(victim->major_list));
	return victim;
}

struct commit *extract_minor(struct commit *victim)
{
	list_del_init(&(victim->list));
	list_del_init(&(victim->major_list));
	return victim;
}

/**
  * display_commit - affiche un commit : "2:  0-2 (stable) 'Work 2'"
  * @c:             commit qui sera affiche
  */
void display_commit(struct commit *c)
{
	printf("%ld:  %2u.%lu %s  %s\n", c->id, c->version.major, c->version.minor, is_unstable_bis(&(c->version)) ? "(unstable)     " : "(stable)	", c->comment);
	
}

/**
  * display_major_commit - affiche un commit majeur: "2:  ### (stable) 'Work 2' ###"
  * @c:             commit qui sera affiche
  */
void display_major_commit(struct commit *c)
{
	printf("%ld:   ### %s  %s ###\n", c->id, is_unstable_bis(&(c->version)) ? "(unstable)     " : "(stable)	", c->comment);
	
}

/**
  * display_history - affiche tout l'historique, i.e. l'ensemble des commits de la liste
  * @from:           premier commit de l'affichage
  */
void display_history(struct history *hist)
{
	printf("Historique de '%s' :\n", hist->name);	
	
	struct commit* c;
	list_for_each_entry(c, &(hist->commit_list->list), list)
	{
			(*c->ops->display)(c);
	}
	
}

/**
  * infos - affiche le commit qui a pour numero de version <major>-<minor>
  * @major: major du commit affiche
  * @minor: minor du commit affiche
  */
void infos(struct commit *from, int major, unsigned long minor)
{
	struct commit* c;
	struct commit* c1;
	
	printf("Recherche du commit %d.%ld :  ",major, minor);

	list_for_each_entry(c, &(from->major_list), major_list)
	{
		if(c->version.major == major) break;
	}
	
	if(c->id != from->id){
		list_for_each_entry(c1, &(c->list), list) {
			if(c1->version.minor == minor){
				display_commit(c1);
				return;
			}
		}
	}
	
	printf("Not Here !!!\n");
	
}

/**
  * commitOf - retourne le commit qui contient la version passe en parametre
  * @version:  pointeur vers la structure version dont cherche le commit
  * Note:      cette fonction continue de fonctionner meme si l'on modifie
  *            l'ordre et le nombre des champs de la structure commit.
  */
struct commit *commitOf(struct version *version)
{
	unsigned long offset;	
	struct commit *com = (struct commit*) malloc (sizeof(struct commit));
	offset = (unsigned long) &(com->version) - (unsigned long) com;
	free (com);
	printf ("offset = %ld, addr = %ld\n", offset, (unsigned long)version);
	return (struct commit *) ((unsigned long)version - offset);
}

void freeHistory(struct commit *from) 
{
	struct commit *c, *p;
	list_for_each_entry_safe(c, p, &(from->list), list) {
		del_commit(c);
		free(c->comment);
		free(c);
	}
	
}

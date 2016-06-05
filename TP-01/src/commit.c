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
	return hist->commit_list->prev;
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
	c->next = c;
	c->prev = c;	
	
	return c;
}

/**
  * insert_commit - insert sans le modifier un commit dans la liste doublement chainee
  * @from:       commit qui deviendra le predecesseur du commit insere
  * @new:        commit a inserer - seul ses champs next et prev seront modifie
  */
static struct commit *insert_commit(struct commit *from, struct commit *new)
{
	new->prev = from;
	new->next = from->next;
	from->next = new;
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
	return insert_commit(from, c);
}

/**
  * del_commit - extrait le commit de l'historique
  * @victim:         commit qui sera sorti de la liste doublement chainee
  */
struct commit *del_commit(struct commit *victim)
{
	/* TODO : Exercice 3 - Question 5 */
  return NULL;
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
  * display_history - affiche tout l'historique, i.e. l'ensemble des commits de la liste
  * @from:           premier commit de l'affichage
  */
void display_history(struct history *hist)
{
	printf("Historique de '%s' :\n", hist->name);	
	
	struct commit* c = hist->commit_list->next; /*car initial = fantome*/	
	while(strcmp(c->comment, "fantome")){
		display_commit(c);
		c = c->next;
	}	
}

/**
  * infos - affiche le commit qui a pour numero de version <major>-<minor>
  * @major: major du commit affiche
  * @minor: minor du commit affiche
  */
void infos(struct commit *from, int major, unsigned long minor)
{
	struct commit* c = from->next;
	
	printf("Recherche du commit %d.%ld :  ",major, minor);
	
	while(c->id != from->id){ 
	
		if((c->version.major == major) && (c->version.minor == minor)){
			display_commit(c);
			return;
		}
		c = c->next;
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

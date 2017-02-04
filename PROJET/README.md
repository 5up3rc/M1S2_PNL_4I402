/******************************************************************************/

/***************                                                ***************/

/***************                  PROJET PNL                    ***************/

/***************     Invité de commande pour le noyau Linux     ***************/

/***************                  04.05.2016                    ***************/

/***************                                                ***************/

/******************************************************************************/


Auteur : ######

2ème membre du binôme : ######


Fichiers fournis :

	- Makefile
	
	- test.c
	
	- projet.c
	
	- projet.h
	
	- README.md


Niveau avancement, développement des fonctions :

	- list
	
	- kill <signal> <pid>
	
	- wait <pid> [<pid> ...]  (entre 1 et 10 processus)
	
	- meminfo
	
	- modinfo <name>
	
	
Manque de temps pour faire fg.

Je ne suis pas super convaincu du passage de paramètres dans test.c , je n'ai pas pu tester autant que je le voulais toutes les fonctionnalités. Cela dit, toute la partie implémentation côté projet.c me parait convaincante.


Utilisation : test option args

  -l, --list            .Afficher la liste des modules exécutés
  
  -k, --kill      	.Tuer un processus       
  
  -w, --wait            .Attendre la mort d'un processus
  
  -m, --meminfo         .Afficher des informations sur la mémoire
  
  -M, --modinfo		.Afficher des informations sur le module   
  
  -?, --help            .Aide   
  
  --usage               .Utilisation    
  
  -V, --version         .Version du programme 
  





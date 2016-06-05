#include "oulipo.h"

void ecrire (char *d)
{
  printf("%ld ecrit '%s'\n", fen, d);
  XChangeProperty(dpy,
		  racine,
		  atomes[0],
		  XA_STRING,
		  8,
		  PropModeReplace,
		  (unsigned char *)d,
		  strlen((char *)d));
}

void lire (char **d)
{
  int                   format;
  unsigned long		lus,restants;
  Atom                  type_obtenu;

  XGetWindowProperty(dpy,
		     racine,
		     atomes[1],
		     0,        /* pas de decalage dans la lecture      */
		     64,       /* multiple de 32 bits, soit 256 octets */
		     False,    /*  detruire une fois la valeur lue     */
		     AnyPropertyType,  /* pas de type precis demande   */
		     &type_obtenu,     /* le type obtenu               */
		     &format,          /* son format 8, 16 ou 32 bits  */
		     &lus,	 /* quantite lue                 */
		     &restants,        /* le reste = offre - demande   */
		     (unsigned char **)d); /* la valeur lue            */
}

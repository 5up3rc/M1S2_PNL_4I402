#include "oulipo.h"

// si on relache la souris dans la fenetre de depart, ne rien faire.
// sinon, attendre la reponse avant de reprendre la statut normal
// (on espere que le relachement a bien eu lieu dans le jumeau)

void fProperty (XPropertyEvent *xpe)
{
  if (xpe->atom == atomes[1])
    {
      int y = ((3*espacement)/4);
      lire(&mot);
      if (recepteur) ecrire(mots[last_sel]); else recepteur = True;
      mots[last_sel]=mot;
      // si le mot effacé etait plus long, faut le faire disparaitre
      XClearWindow(dpy, filles[last_sel]); 
      reecrire(gc, filles[last_sel], espacement, y, mot);
    }
}

void fButtonPress(XButtonEvent *xbe)
{
  recepteur = False;
}

void fButtonRelease(XButtonEvent *xbe)
{
  if (!xbe->subwindow) ecrire(mots[last_sel]); 
  else recepteur = True;
}

void fMotion(XButtonEvent *xbe)
{}

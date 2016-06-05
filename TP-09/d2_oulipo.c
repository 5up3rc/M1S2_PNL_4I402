#include "oulipo.h"
Window synchroniser (char *s1, char *s2)
{
  XEvent		e;
  int			numprog;
  Window		w1,w2;
  Atom			sync = XInternAtom(dpy, s1, False);  

  XGrabServer(dpy);
  w1 = XGetSelectionOwner(dpy, sync);
  XSetSelectionOwner(dpy, sync, fen, CurrentTime);
  XUngrabServer(dpy);

  for (;;) {
    XNextEvent(dpy, &e);
    if ((e.type == SelectionClear) && (e.xselectionclear.selection == sync))
       break;
  }
  /* Variante qui n'elimine pas les autres evenements sans les traiter. */
  /* Mais en l'absence de la fonction XTypedEvent qui serait bloquante */
  /* on se retrouve avec une boucle qui meriterait un Sleep */
  /*
  while (!XCheckTypedEvent(dpy, SelectionClear, &e)) {
    if (e.xselectionclear.selection == sync))
       break;
  }
  */
  w2 = XGetSelectionOwner(dpy, sync);
  numprog = (w2 == None);

  if   (!numprog)
       XSetSelectionOwner(dpy, sync, None, CurrentTime);
  else w2 = w1;
 
  atomes[1-numprog]   = sync;
  atomes[numprog]     = XInternAtom(dpy, s2, False);

  return w2;
}

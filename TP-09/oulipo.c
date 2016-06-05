#include "oulipo.h"

Display          *dpy;
Window            fen, autrefen, racine, filles[nb_LIGNES];
GC                gc,gcinv;
Atom 		  atomes[2];
XFontStruct      *font;
Bool              recepteur;
int		  nb_mots;
char 		  **mots;
char 		  *mot;
int		  last_sel=0;
int		espacement,largeur;

  /* utile au TME seulement */
Window		drag=0; 
int x_root=0; 
int y_root=0;

int main (int argc, char * argv[])
{
  XEvent e;

  nb_mots=argc-3;
  if ((nb_mots >= nb_LIGNES) || nb_mots <= 0)
    {fprintf(stderr, "au moins 3 et pas plus de %d d'arguments\n", nb_LIGNES);
     exit(1);}

  if ((dpy = XOpenDisplay(NULL)) == NULL)
     {fprintf(stderr, "%s: dispositif %s injoignable\n", argv[0],
	      XDisplayName(NULL));
      exit(2);}

  racine = DefaultRootWindow(dpy);

  creerGC(argv[1]);

  fen = XCreateSimpleWindow(dpy, racine, 0, 0, 
			    largeur, (nb_mots+2)*(espacement), 0,
			    BlackPixel(dpy,DefaultScreen(dpy)),
			    WhitePixel(dpy,DefaultScreen(dpy)));

  XStoreName(dpy, fen, argv[2]);
  autrefen = synchroniser("interpelle", "répond");
 
  sensibiliser();

  XMapWindow(dpy, fen);
  XMapSubwindows(dpy, fen);

  recepteur = True;
  mots = &argv[3];

  for (;;) {
    XNextEvent(dpy, &e);
    switch(e.type) {
    case Expose :      fExpose(&e.xexpose); break;
    case KeyPress :    fKeyPress(&e.xkey); break;
    case EnterNotify : fEnterNotify(&e.xcrossing); break;
    case LeaveNotify : fLeaveNotify(&e.xcrossing); break;
    case PropertyNotify : fProperty (&e.xproperty); break;
    case ButtonPress : fButtonPress(&e.xbutton);   break;
    case ButtonRelease : fButtonRelease(&e.xbutton);   break;
    case MotionNotify : fMotion(&e.xbutton);   break;
    default : break;
    }
  }
}

void fKeyPress (XKeyEvent *e)
{
      printf("fenetre %lu en mode %d arrete\n", e->window, e->send_event);
      if (!e->send_event)
	{ XSendEvent(dpy,autrefen, False, KeyPressMask, (XEvent *) e);}
      XDeleteProperty(dpy,racine, atomes[0]);
      XFreeGC(dpy, gc);
      XFreeGC(dpy, gcinv);
      XCloseDisplay(dpy);
      printf("%lu en mode %d arrete\n", e->window, e->send_event);
      exit(0);
}


void creerGC (char *f)
{
  XGCValues   xgcv;

  if (!(font = XLoadQueryFont(dpy, f)))
    {fprintf(stderr, "police %s inconnue.\n", f);exit(1);}

  espacement = (font->ascent + font->descent)<<1;
  largeur    = espacement<<3;

  xgcv.font = font->fid;

  xgcv.foreground = BlackPixel(dpy, DefaultScreen(dpy));
  xgcv.background = WhitePixel(dpy, DefaultScreen(dpy));
  gc = XCreateGC(dpy, racine, GCForeground|GCBackground|GCFont, &xgcv);

  xgcv.foreground = WhitePixel(dpy, DefaultScreen(dpy));
  xgcv.background = BlackPixel(dpy, DefaultScreen(dpy));
  gcinv = XCreateGC(dpy, racine, GCForeground|GCBackground|GCFont, &xgcv);
}

void fExpose (XExposeEvent *xee)
{
  int i, y;

  y = ((3*espacement)/4);
  if (!xee->count)
    for (i = 0; i < nb_mots ; i++)
      reecrire(gc, filles[i], espacement, y, mots[i]);
}

void reecrire (GC g, Window w, int x, int y, char *mot)
{
  if (mot) 
    XDrawImageString(dpy, w, g, x, y, mot, strlen(mot));
}

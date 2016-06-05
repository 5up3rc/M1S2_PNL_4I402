#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#define nb_LIGNES  12
extern Display          *dpy;
extern Window            fen, autrefen, racine, filles[nb_LIGNES];
extern GC                gc,gcinv;
extern Atom 		  atomes[2];
extern XFontStruct      *font;
extern Bool              recepteur;
extern int		  nb_mots;
extern char 		  **mots;
extern char 		  *mot;
extern int		  last_sel;
extern int		espacement,largeur;
extern Window		drag;
extern int x_root;
extern int y_root;
extern int main (int argc, char * argv[]);
extern void fKeyPress (XKeyEvent *e);
extern void creerGC (char *f);
extern void fExpose (XExposeEvent *xee);
extern void reecrire (GC g, Window w, int x, int y, char *mot);
extern void sensibiliser();
extern Window synchroniser (char *s1, char *s2);
extern void fLeaveNotify (XCrossingEvent *e);
extern void fEnterNotify (XCrossingEvent *e);
extern void ecrire (char *d);
extern void lire (char **d);
extern void fProperty (XPropertyEvent *xpe);
extern void fButtonPress(XButtonEvent *xbe);
extern void fButtonRelease(XButtonEvent *xbe);
extern void fMotion(XButtonEvent *xbe);

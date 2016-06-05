#include "oulipo.h"


void sensibiliser()
{
  int i;

  for (i= 0 ; i < nb_mots;i++)
      {filles[i] = XCreateSimpleWindow(dpy, fen,
				       0, (i+1)*espacement,
				       largeur, espacement, 0,
				       BlackPixel(dpy,DefaultScreen(dpy)),
				       WhitePixel(dpy,DefaultScreen(dpy)));
      XSelectInput(dpy, filles[i],  EnterWindowMask|LeaveWindowMask); }

  XSelectInput(dpy, racine, PropertyChangeMask);

  XSelectInput(dpy, fen, KeyPressMask|ExposureMask);

  XSelectInput(dpy, autrefen, ButtonReleaseMask|ButtonPressMask|OwnerGrabButtonMask);
}

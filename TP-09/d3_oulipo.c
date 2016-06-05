#include "oulipo.h"

void fLeaveNotify (XCrossingEvent *e)
{
 if ((!recepteur) || (e->mode==NotifyNormal))
 {
   int y = ((3*espacement)/4);
   reecrire(gc, filles[last_sel], espacement, y, mots[last_sel]);
 }
}

void fEnterNotify (XCrossingEvent *e)
{
   if ((!recepteur) || (e->mode==NotifyNormal))
     {
       for (last_sel = 0 ; last_sel < nb_mots ; last_sel++) {
	 if (filles[last_sel] == e->window) {
	   int y = ((3*espacement)/4);
	   reecrire(gcinv, e->window, espacement, y, mots[last_sel]);
	   break;
	 }
       }
     }
}

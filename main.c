#include "Xstuff.h"
#include "proto.h"

void draw_main(int expose,windowstuff *win)
  {
  main_desc *md = win->local->desc.main;
  
  xclear(md->w);
  xstring(md->w,md->panex[0],
          BORDER+md->fontinfo->ascent,md->name[0],
          strlen(md->name[0]));
  xstring(md->w,md->panex[1],
          BORDER+md->fontinfo->ascent,md->name[1],
          strlen(md->name[1]));
  }

windowstuff *create_main(windowstuff *rw,int width,int height,
                         XFontStruct *fontinfo,
                         char *name1,char *name2,
                         int pane1x,int pane2x)
  {
  windowstuff *rv;
  char winname[LINELEN];

  sprintf(winname,"bxdiff: %s vs. %s",name1,name2);

  rv = openwindow(rw,winname,PPosition,0,0,
                  width,height,1,0,
                  ExposureMask | KeyPressMask | StructureNotifyMask,
                  draw_main);
  assoc_win(rv);
  rv->local = newitem(local_type,1);
  rv->local->object = MAIN;
  rv->local->desc.main = newitem(main_desc,1);
  rv->local->desc.main->w = rv;
  rv->local->desc.main->fontinfo = fontinfo;
  rv->local->desc.main->name[0]=name1;
  rv->local->desc.main->name[1]=name2;
  rv->local->desc.main->panex[0]=pane1x;
  rv->local->desc.main->panex[1]=pane2x;
  
  XSetFont(rv->display,rv->gc,fontinfo->fid);
  draw_main(0,rv);
  return(rv);
  }

void destroy_main(windowstuff **rv)
  {
  disassoc_win((*rv));
  free((*rv)->local->desc.main);
  free((*rv)->local);
  closewindow(*rv);
  *rv = NULL;
  }


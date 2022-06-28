#include "Xstuff.h"
#include "proto.h"

void draw_xover(int expose,windowstuff *win)
  {
  XPoint points[9];
  int i,w,h;
  int a1,a2,a3,a4;
  int wh;
  xover_desc *xover = win->local->desc.xover;
  
  xclear(xover->w);
  w = xover->w->hint.width;
  wh = xover->w->hint.height;
  
  h = xover->h;
  
  for (i=0;i<xover->dif->diffs;i++)
    {
    a1=(xover->dif->diff[i].fs[0]-xover->start[0])*h;
    a2=(xover->dif->diff[i].fs[1]-xover->start[1])*h;
    a3=(xover->dif->diff[i].fe[1]-xover->start[1])*h;
    a4=(xover->dif->diff[i].fe[0]-xover->start[0])*h;

    if (a1<0 && a2<0 && a3<0 && a4<0) continue;
    if (a1>wh && a2>wh && a3>wh && a4>wh) continue;

    if (a1<0) a1=0;
    if (a2<0) a2=0;
    if (a3<0) a3=0;
    if (a4<0) a4=0;

    if (a1>wh) a1=wh;
    if (a2>wh) a2=wh;
    if (a3>wh) a3=wh;
    if (a4>wh) a4=wh;

    points[0].x = 0;
    points[0].y = a1; 
    points[1].x = w;
    points[1].y = a2;
    points[2].x = w;
    points[2].y = a3;
    points[3].x = 0;
    points[3].y = a4;
    points[4].x = 0;
    points[4].y = points[0].y;

    XFillPolygon(xover->w->display,xover->w->window,xover->w->gc,
                 points,5,Nonconvex,CoordModeOrigin);
    XDrawLines(xover->w->display,xover->w->window,xover->w->gc,
                 points,5,CoordModeOrigin);
    
    }

  XSetFunction(xover->w->display,xover->w->gc,FunnyMode);
  XSetForeground(xover->w->display,xover->w->gc,
                 xover->w->foreground|xover->w->background);
  
  for (int s=0;s<2;s++)
    {
    int x = s ? w-1 : 1;
    int d = s ? -1 : 1;
    
    for (i=0;i<xover->dif->file[s].lines;i++)
      if (xover->dif->file[s].matches[i])
        {
        int y = h*(i-xover->start[s])+h/2;
        if (y>=0 && y<=wh)
          draw_marker(xover->w,x,y,d);
        }
    }
  

  draw_arrow(xover->w,1,(xover->cur[0]-xover->start[0])*h+h/2,1);
  draw_arrow(xover->w,w-1,(xover->cur[1]-xover->start[1])*h+h/2,-1);
  pencolor(xover->w,1);
  XSetFunction(xover->w->display,xover->w->gc,GXcopy);
  }

windowstuff *create_xover(windowstuff *main,int x,int y,int w,int l,
                          diffs *dfs,int fh)
  {
  windowstuff *rv;
  
  rv = openwindow(main,"Map",PPosition,x,y,w,l,-1,0,
                  ExposureMask|StructureNotifyMask,
                  draw_xover);
  assoc_win(rv);
  rv->local = newitem(local_type,1);
  rv->local->object = XOVER;
  rv->local->desc.xover = newitem(xover_desc,1);
  rv->local->desc.xover->w = rv;
  rv->local->desc.xover->dif = dfs;
  rv->local->desc.xover->start[0]=0;
  rv->local->desc.xover->start[1]=0;
  rv->local->desc.xover->cur[0]=0;
  rv->local->desc.xover->cur[1]=0;
  rv->local->desc.xover->h=fh;
  
  draw_xover(0,rv);
  return(rv);
  }

void destroy_xover(windowstuff **rv)
  {
  disassoc_win((*rv));
  free((*rv)->local->desc.xover);
  free((*rv)->local);
  closewindow(*rv);
  *rv = NULL;
  }


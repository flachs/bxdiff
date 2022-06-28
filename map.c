#include "Xstuff.h"
#include "proto.h"

void draw_map(int expose,windowstuff *win)
  {
  map_desc *map = win->local->desc.map;
  XPoint points[9];
  int s,i,m,h;

  m = max(map->dif->file[0].lines,map->dif->file[1].lines);
  h = map->w->hint.height;
  
  if (expose)
    {
    xclear(map->w);

    for (i=0;i<map->dif->diffs;i++)
      {
      points[0].x = BORDER;
      points[0].y = h*map->dif->diff[i].fs[0]/m;
      points[1].x = MAPWID/3;
      points[1].y = h*map->dif->diff[i].fs[0]/m;
      points[2].x = 2*MAPWID/3;
      points[2].y = h*map->dif->diff[i].fs[1]/m;
      points[3].x = MAPWID-BORDER;
      points[3].y = h*map->dif->diff[i].fs[1]/m;
      points[4].x = MAPWID-BORDER;
      points[4].y = h*map->dif->diff[i].fe[1]/m;
      points[5].x = 2*MAPWID/3;
      points[5].y = h*map->dif->diff[i].fe[1]/m;
      points[6].x = MAPWID/3;
      points[6].y = h*map->dif->diff[i].fe[0]/m;
      points[7].x = BORDER;
      points[7].y = h*map->dif->diff[i].fe[0]/m;
      points[8].x = BORDER;
      points[8].y = h*map->dif->diff[i].fs[0]/m;
    
      XFillPolygon(map->w->display,map->w->window,map->w->gc,
                   points,9,Nonconvex,CoordModeOrigin);
      XDrawLines(map->w->display,map->w->window,map->w->gc,
                 points,9,CoordModeOrigin);
    
      }
    for (s=0;s<2;s++)
      {
      int x = s ? MAPWID-BORDER : BORDER;
      int d = s ? 1 : -1;
      
      for (i=0;i<map->dif->file[s].lines;i++)
        if (map->dif->file[s].matches[i])
          draw_marker(map->w,x,h*i/m,d);
      }
    }
  else if (map->pcur[0]>=0)
    {
    XSetFunction(map->w->display,map->w->gc,FunnyMode);
    XSetForeground(map->w->display,map->w->gc,map->w->foreground|map->w->background);
    draw_arrow(map->w,BORDER,h*map->pcur[0]/m,-1);
    draw_arrow(map->w,MAPWID-BORDER,h*map->pcur[1]/m,1);
    pencolor(map->w,1);
    XSetFunction(map->w->display,map->w->gc,GXcopy);
    }
  
  XSetFunction(map->w->display,map->w->gc,FunnyMode);
  XSetForeground(map->w->display,map->w->gc,map->w->foreground|map->w->background);
  draw_arrow(map->w,BORDER,h*map->cur[0]/m,-1);
  draw_arrow(map->w,MAPWID-BORDER,h*map->cur[1]/m,1);
  pencolor(map->w,1);
  XSetFunction(map->w->display,map->w->gc,GXcopy);
  map->pcur[0]=map->cur[0];
  map->pcur[1]=map->cur[1];
  }

windowstuff *create_map(windowstuff *main,int x,int y,int w,int l,diffs *dfs)
  {
  windowstuff *rv;
  
  rv = openwindow(main,"Map",PPosition,x,y,w,l,-1,0,
                  ExposureMask|StructureNotifyMask| ButtonPressMask |
                  Button1MotionMask | PointerMotionHintMask |
                  ButtonReleaseMask,
                  draw_map);
  assoc_win(rv);
  rv->local = newitem(local_type,1);
  rv->local->object = MAP;
  rv->local->desc.map = newitem(map_desc,1);
  rv->local->desc.map->w = rv;
  rv->local->desc.map->dif = dfs;
  rv->local->desc.map->cur[0]=0;
  rv->local->desc.map->cur[1]=0;
  rv->local->desc.map->pcur[0]= -1;
  rv->local->desc.map->pcur[1]= -1;
  draw_map(0,rv);
  return(rv);
  }

void destroy_map(windowstuff **rv)
  {
  disassoc_win((*rv));
  free((*rv)->local->desc.map);
  free((*rv)->local);
  closewindow(*rv);
  *rv = NULL;
  }



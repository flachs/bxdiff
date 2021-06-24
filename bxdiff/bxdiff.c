#include "Xstuff.h"
#include "proto.h"

#define FunnyMode GXxor
#define xistring(w,x,y,s,l) XDrawImageString((w)->display,(w)->window,(w)->gc,(int)(x),(int)(y),s,(int)(l))

void popitup(diffs *dfs)
  {
  windowstuff *rw,*main,*eventwind,*pane[2],*map,*xover;
  char text[10];
  KeySym  mykey;
  XFontStruct *fontinfo;
  int numlines,panew[2],panelen,panex[2];
  int done;
  int i;
  int r_x,r_y,p_x,p_y;
  unsigned int k_b;
  Window rwind,rchild;
  
  /* establish root connection */
  rw = openroot(NULL);

  if (!rw)
    {
    fprintf(stderr,"bxdiff: could not open display\n");
    exit(1);
    }

  /* figure out font stuff */
  fontinfo = XLoadQueryFont(rw->display,"6x10");
  if (! fontinfo)
    {
    fprintf(stderr,"bxdiff: cant load font %s\n","6x10");
    exit(1);
    }
 
  /* create main window */
  if (dfs->file[0].width>MAXPANEWID) dfs->file[0].width=MAXPANEWID;
  if (dfs->file[1].width>MAXPANEWID) dfs->file[1].width=MAXPANEWID;

  numlines = min(max(dfs->file[0].lines,dfs->file[1].lines),80);

  panex[0] = BORDER;
  panew[0] = dfs->file[0].width*fontinfo->max_bounds.width+2*INDENT;
  panex[1] = panex[0]+panew[0]+XOVERWID;
  panew[1] = dfs->file[1].width*fontinfo->max_bounds.width+2*INDENT;

  panelen = numlines*(fontinfo->ascent+fontinfo->descent);
  
  main = create_main(rw,3*BORDER+panew[0]+panew[1]+XOVERWID+MAPWID,
                     2*BORDER+panelen+fontinfo->ascent+fontinfo->descent,
                     fontinfo,
                     dfs->file[0].name,dfs->file[1].name,
                     panex[0],panex[1]);

  /* create text panes */
  for (i=0;i<2;i++)
    {
    pane[i] = create_pane(main,panex[i],fontinfo->ascent+fontinfo->descent+BORDER,
                          panew[i],panelen,i,dfs,fontinfo);
    }

  /* create map */
  map = create_map(main,panex[1]+panew[1]+BORDER,
                   fontinfo->ascent+fontinfo->descent+BORDER,
                   MAPWID,panelen,dfs);

  /* create xover */
  xover = create_xover(main,panex[0]+panew[0]+BORDER,
                       fontinfo->ascent+fontinfo->descent+BORDER,
                       panex[1]-BORDER-(panex[0]+panew[0]+BORDER),
                       panelen,dfs,
                       fontinfo->ascent+fontinfo->descent);

  /* finally the event loop */
  for (done=0; !done ; )
    {
    XNextEvent(main->display,&main->event);
    eventwind = find_win(main->event.xexpose.window) ;
    if (! eventwind) continue;
    eventwind->event = main->event;
    switch (main->event.type)
      {
      case Expose:
        if (main->event.xexpose.count == 0)
          draw_object(1,eventwind);
        break;
      case ConfigureNotify:
        eventwind->hint.height = eventwind->event.xconfigure.height;
        eventwind->hint.width = eventwind->event.xconfigure.width;
        if (eventwind->local->object == MAIN)
          {
          numlines=(eventwind->event.xconfigure.height-
                    2*BORDER-fontinfo->ascent-fontinfo->descent)/
                   (fontinfo->ascent+fontinfo->descent);
          panelen = numlines*(fontinfo->ascent+fontinfo->descent);
          main->local->desc.main->panex[0]=panex[0] = BORDER;
          panew[0] = (eventwind->event.xconfigure.width-3*BORDER-XOVERWID-MAPWID)/2;
          main->local->desc.main->panex[1]=panex[1] = panex[0]+panew[0]+XOVERWID;
          panew[1] = panew[0];
          
          XMoveResizeWindow(pane[0]->display,pane[0]->window,
                            panex[0],fontinfo->ascent+fontinfo->descent+BORDER,
                            panew[0],panelen);
          XMoveResizeWindow(pane[1]->display,pane[1]->window,
                            panex[1],fontinfo->ascent+fontinfo->descent+BORDER,
                            panew[1],panelen);
          XMoveResizeWindow(map->display,map->window,
                            panex[1]+panew[1]+BORDER,
                            fontinfo->ascent+fontinfo->descent+BORDER,
                            MAPWID,panelen);
          XMoveResizeWindow(xover->display,xover->window,
                            panex[0]+panew[0]+BORDER,
                            fontinfo->ascent+fontinfo->descent+BORDER,
                            panex[1]-BORDER-(panex[0]+panew[0]+BORDER),
                            panelen);
          }
        break;
      case MotionNotify:
        while (XCheckMaskEvent(eventwind->display,PointerMotionMask,
               &eventwind->event));
      case ButtonPress:
      case ButtonRelease:
        if (!XQueryPointer(eventwind->display,eventwind->event.xmotion.window,
                           &rwind,&rchild,&r_x,&r_y,&p_x,&p_y,&k_b))
          {
          break;
          }
        gotomouse(eventwind,p_x,p_y);
        motion_sync(map,xover,pane,numlines);
        break;
      case KeyPress:
        i = XLookupString((XKeyEvent *)&main->event, text, 10, &mykey, 0);
        if (i==1)
          {
          switch (text[0])
            {
            case 'q': /* quit */
              done=1;
              break;
            case 'g': /* to the top */
              gototop(map);
              motion_sync(map,xover,pane,numlines);
              break;
            case 'G': /* down */
              gotobot(map);
              motion_sync(map,xover,pane,numlines);
              break;
            case 'j': /* down */
              movedown(map);
              motion_sync(map,xover,pane,numlines);
              break;
            case ' ': /* page down */
              for (i=0;i<numlines/2;i++) movedown(map);
              motion_sync(map,xover,pane,numlines);
              break;
            case 'k': /* up */
              moveup(map);
              motion_sync(map,xover,pane,numlines);
              break;
            case 'b': /* page up */
              for (i=0;i<numlines/2;i++) moveup(map);
              motion_sync(map,xover,pane,numlines);
              break;
            case 'n': /* find next diff */
              while (movedown(map)) ;
              motion_sync(map,xover,pane,numlines);
              break;
            case 'p': /* find pref diff */
              while (moveup(map)) ;
              motion_sync(map,xover,pane,numlines);
              break;
            }
          }
      }
    }

  destroy_xover(&xover);
  destroy_map(&map);

  for (i=0;i<2;i++) destroy_pane(&pane[i]);
  
  destroy_main(&main);
  XFreeFont(rw->display,fontinfo);
  closeroot(rw);
  }

void motion_sync(windowstuff *map,windowstuff *xover,windowstuff **pane,int numlines)
  {
  draw_object(0,map);
  xover->local->desc.xover->cur[0]=map->local->desc.map->cur[0];
  xover->local->desc.xover->cur[1]=map->local->desc.map->cur[1];
  xover->local->desc.xover->start[0]=map->local->desc.map->cur[0]
                                     -numlines/2;
  if (xover->local->desc.xover->start[0]<0)
    xover->local->desc.xover->start[0]=0;
              
  xover->local->desc.xover->start[1]=map->local->desc.map->cur[1]
                                     -numlines/2;
  if (xover->local->desc.xover->start[1]<0)
    xover->local->desc.xover->start[1]=0;
  draw_object(0,xover);

  pane[0]->local->desc.pane->startline
    = xover->local->desc.xover->start[0];
  pane[1]->local->desc.pane->startline
    = xover->local->desc.xover->start[1];

  draw_object(0,pane[0]);
  draw_object(0,pane[1]);
  }

static int srch= -2;
void gototop(windowstuff *mapw)
  {
  map_desc *map;
  
  map=mapw->local->desc.map;
  map->cur[0]=0;
  map->cur[1]=0;
  if (srch!=-2)
    srch = map->dif->dr0[map->cur[0]] & map->dif->dr1[map->cur[1]];
  }

void gotobot(windowstuff *mapw)
  {
  map_desc *map;

  map=mapw->local->desc.map;
  map->cur[0]=map->dif->file[0].lines-1;
  map->cur[1]=map->dif->file[1].lines-1;
  if (srch!=-2)
    srch = map->dif->dr0[map->cur[0]] & map->dif->dr1[map->cur[1]];
  }

int moveup(windowstuff *mapw)
  {
  int nl1,nl0,c0,c1,dr;
  map_desc *map;

  if (mapw->local->object!=MAP)
    {
    fprintf(stderr,"bxdiff: internal error ... non map passed to moveup\n");
    exit(1);
    }
  
  map=mapw->local->desc.map;
  c0=map->cur[0]-1;
  if (c0<0) c0=0;
  nl1=map->dif->l01[c0];
  
  c1=map->cur[1]-1;
  if (c1<0) c1=0;
  nl0=map->dif->l10[c1];

  if (nl0<c0 && nl1>=c1)
    {
    map->cur[0]=c0;
    map->cur[1]=nl1;
    }
  else if (nl1<c1 && nl0>=c0)
    {
    map->cur[0]=nl0;
    map->cur[1]=c1;
    }
  else
    {
    map->cur[0]=c0;
    map->cur[1]=c1;
    }

  dr = map->dif->dr0[map->cur[0]] & map->dif->dr1[map->cur[1]];

  if (map->cur[0]==0 && map->cur[1]==0)
    {
    srch=dr;
    return(0);
    }
  
  if (srch==-2 || srch==dr || dr==-1)
    {
    srch=dr;
    return(1);
    }

  srch=dr;
  return(0);
  }

int movedown(windowstuff *mapw)
  {
  int nl1,nl0,c0,c1,dr;
  map_desc *map;

  if (mapw->local->object!=MAP)
    {
    fprintf(stderr,"bxdiff: internal error ... non map passed to movedown\n");
    exit(1);
    }
  
  map=mapw->local->desc.map;
  c0=map->cur[0]+1;
  if (c0>=map->dif->file[0].lines) c0=map->dif->file[0].lines-1;
  nl1=map->dif->l01[c0];
  
  c1=map->cur[1]+1;
  if (c1>=map->dif->file[1].lines) c1=map->dif->file[1].lines-1;
  nl0=map->dif->l10[c1];

  if (nl0>c0 && nl1<=c1 && c0>map->cur[0])
    {
    map->cur[0]=c0;
    map->cur[1]=nl1;
    }
  else if (nl1>c1 && nl0<=c0 && c1>map->cur[1])
    {
    map->cur[0]=nl0;
    map->cur[1]=c1;
    }
  else
    {
    map->cur[0]=c0;
    map->cur[1]=c1;
    }

  dr = map->dif->dr0[map->cur[0]] & map->dif->dr1[map->cur[1]];

  if (map->cur[0]==map->dif->file[0].lines-1 &&
      map->cur[1]==map->dif->file[1].lines-1)
    {
    srch=dr;
    return(0);
    }
  
  if (srch==-2 || srch==dr || dr==-1)
    {
    srch=dr;
    return(1);
    }

  srch=dr;
  return(0);
  }

void destroy_xover(windowstuff **rv)
  {
  disassoc_win((*rv));
  free((*rv)->local->desc.xover);
  free((*rv)->local);
  closewindow(*rv);
  *rv = NULL;
  }

void draw_arrow(windowstuff *w,int x,int y,int dir)
  {
  static XPoint points[3]={ 0,0, 4,-3, 0,6 };

  points[0].x=x;
  points[0].y=y;
  points[1].x *= dir;
  
  XFillPolygon(w->display,w->window,w->gc,
               points,3,Nonconvex,CoordModePrevious);
  points[1].x *= dir;
  }

void draw_xover(int expose,xover_desc *xover)
  {
  XPoint points[9];
  int i,w,h;
  int a1,a2,a3,a4;
  int wh;
  
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
  XSetForeground(xover->w->display,xover->w->gc,xover->w->foreground|xover->w->background);
  draw_arrow(xover->w,1,(xover->cur[0]-xover->start[0])*h+h/2,1);
  draw_arrow(xover->w,w-1,(xover->cur[1]-xover->start[1])*h+h/2,-1);
  pencolor(xover->w,1);
  XSetFunction(xover->w->display,xover->w->gc,GXcopy);
  }

windowstuff *create_xover(windowstuff *main,int x,int y,int w,int l,diffs *dfs,int fh)
  {
  windowstuff *rv;
  
  rv = openwindow(main,"Map",PPosition,x,y,w,l,-1,0,
                  ExposureMask|StructureNotifyMask);
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
  
  draw_map(0,rv->local->desc.map);
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

void gotomouse(windowstuff *w,int x,int y)
  {
  int i,m,h,l;
  map_desc *map;

  if (w->local->object!=MAP)
    {
    fprintf(stderr,"bxdiff: internal error ... gotomouse given non map argument\n");
    exit(1);
    }

  map = w->local->desc.map;
  
  m = max(map->dif->file[0].lines,map->dif->file[1].lines);
  h = map->w->hint.height;

  l = (m*y)/h;

  if (x>w->hint.width/2)
    {
    if (l<0) l=0;
    if (l>=map->dif->file[1].lines) l=map->dif->file[1].lines-1;
    map->cur[0]=map->dif->l10[l];
    if (map->cur[0]<0) map->cur[0]=0;
    if (map->cur[0]>=map->dif->file[0].lines) map->cur[0]=map->dif->file[0].lines-1;
    map->cur[1]=l;
    }
  else
    {
    if (l<0) l=0;
    if (l>=map->dif->file[0].lines) l=map->dif->file[0].lines-1;
    map->cur[0]=l;
    map->cur[1]=map->dif->l01[l];
    if (map->cur[1]<0) map->cur[1]=0;
    if (map->cur[1]>=map->dif->file[1].lines) map->cur[1]=map->dif->file[1].lines-1;
    }

  if (srch!=-2)
    srch = map->dif->dr0[map->cur[0]] & map->dif->dr1[map->cur[1]];
  }

void draw_map(int expose,map_desc *map)
  {
  XPoint points[9];
  int i,m,h;

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
                  Button1MotionMask | PointerMotionHintMask | ButtonReleaseMask );
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
  draw_map(0,rv->local->desc.map);
  return(rv);
  }


void destroy_pane(windowstuff **rv)
  {
  disassoc_win((*rv));
  free((*rv)->local->desc.pane);
  free((*rv)->local);
  closewindow(*rv);
  *rv = NULL;
  }

void rendertabs(char *in,char *out)
  {
  int i;
  char *p,*q;

  i=0;

  for (p=in,q=out; *p ; p++)
    {
    if (*p=='\t')
      {
      do 
        {
        *q++ = ' ' ;
        i++;
        }
      while (i%8);
      }
    else
      {
      *q++ = *p ;
      i++;
      }
    }
  *q=0;
  }

void draw_pane(int expose,pane_desc *pd)
  {
  int i,l,fl,height,b,j,ia,start,end,rm;
  char **line,*theline;
  char buffer[LINELEN];

  height = (pd->fontinfo->ascent+pd->fontinfo->descent);
  fl = pd->w->hint.height/height;
  line = pd->dif->file[pd->side].line;
  l=pd->dif->file[pd->side].lines;

  if (expose) pd->pstartline= -1;

  rm = pd->startline-pd->pstartline;
  
  if (rm == 0) return;

  if (fl-rm>0 && rm>0 && pd->pstartline>=0)
    {
    XCopyArea(pd->w->display,pd->w->window,pd->w->window,pd->w->gc,
              0,rm*height,
              pd->w->hint.width,(fl-rm)*height,
              0,0);
    XClearArea(pd->w->display,pd->w->window,
               0,(fl-rm)*height,
               pd->w->hint.width,rm*height,
               False);
    start=fl-rm;
    end=fl;
    }
  else if (fl+rm>0 && rm<0 && pd->pstartline>=0)
    {
    XCopyArea(pd->w->display,pd->w->window,pd->w->window,pd->w->gc,
              0,0,
              pd->w->hint.width,(fl+rm)*height,
              0,-rm*height);
    XClearArea(pd->w->display,pd->w->window,
               0,0,
               pd->w->hint.width,-rm*height,
               False);
    start=0;
    end= -rm;
    }
  else
    {
    xclear(pd->w);
    start=0;
    end=fl;
    }
  
  pd->pstartline=pd->startline;
    
  for (i=start;i<end;i++)
    {
    if (i+pd->startline<l && i+pd->startline>=0 ) theline=line[i+pd->startline];
    else theline="~";

    b=0;
    ia=0;
    for (j=0;j<pd->dif->diffs;j++)
      {
      b |= pd->dif->diff[j].fs[pd->side]<=i+pd->startline
           && pd->dif->diff[j].fe[pd->side]>i+pd->startline ;
      ia |= pd->dif->diff[j].fs[pd->side]<=i+pd->startline
            && pd->dif->diff[j].fe[pd->side]==i+pd->startline ;
      }

    if (theline[0])
      {
      rendertabs(theline,buffer);
      xistring(pd->w,INDENT,i*height+pd->fontinfo->ascent,
               buffer,strlen(buffer));
      }
    
    if (b)
      {
      XSetFunction(pd->w->display,pd->w->gc,FunnyMode);
      XSetForeground(pd->w->display,pd->w->gc,pd->w->foreground|pd->w->background);
      XFillRectangle(pd->w->display,pd->w->window,pd->w->gc,0,i*height,
                     pd->w->hint.width,height);
      pencolor(pd->w,1);
      XSetFunction(pd->w->display,pd->w->gc,GXcopy);
      }

    if (ia)
      {
      XSetFunction(pd->w->display,pd->w->gc,FunnyMode);
      XSetForeground(pd->w->display,pd->w->gc,pd->w->foreground|pd->w->background);
      XDrawLine(pd->w->display,pd->w->window,pd->w->gc,0,i*height,
                     pd->w->hint.width,i*height);
      pencolor(pd->w,1);
      XSetFunction(pd->w->display,pd->w->gc,GXcopy);
      }
    }
  }

windowstuff *create_pane(windowstuff *main,int x,int y,int w,int l,
                         int id,diffs *dfs,XFontStruct *fontinfo)
  {
  windowstuff *rv;
  
  rv = openwindow(main,"Text Pane",PPosition,x,y,w,l,-1,0,
                  ExposureMask|StructureNotifyMask);
  assoc_win(rv);
  rv->local = newitem(local_type,1);
  rv->local->object = PANE;
  rv->local->desc.pane = newitem(pane_desc,1);
  rv->local->desc.pane->w = rv;
  rv->local->desc.pane->fontinfo = fontinfo;
  rv->local->desc.pane->side = id;
  rv->local->desc.pane->dif = dfs;
  rv->local->desc.pane->startline=0;
  rv->local->desc.pane->pstartline= -1;
  XSetFont(rv->display,rv->gc,fontinfo->fid);
  draw_pane(0,rv->local->desc.pane);
  return(rv);
  }



void draw_main(int expose,main_desc *md)
  {
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
                 ExposureMask | KeyPressMask | 
                 StructureNotifyMask );
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
  draw_main(0,rv->local->desc.main);
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

void draw_object(int expose,windowstuff *eventwind)
  {
  switch (eventwind->local->object)
    {
    case MAIN: 
      draw_main(expose,eventwind->local->desc.main);
      break;
    case PANE:
      draw_pane(expose,eventwind->local->desc.pane);
      break;
    case MAP:
      draw_map(expose,eventwind->local->desc.map);
      break;
    case XOVER :
      draw_xover(expose,eventwind->local->desc.xover);
      break;
    }
  }


void printdiffs(diffs *dfs)
  {
  int i,j;
  
  for (i=0;i<2;i++)
    {
    printf("file %d: %s\n",i,dfs->file[i].name);
    printf("  %d lines %d chars %d wide\n",dfs->file[i].lines,dfs->file[i].len,
           dfs->file[i].width);

    for (j=0;j<dfs->file[i].lines;j++)
      {
      printf("%d:%04d> %s\n",i,j,dfs->file[i].line[j]);
      }
    }

  for (i=0;i<dfs->diffs;i++)
    {
    printf("%d %d %c %d %d\n",dfs->diff[i].fs[0],dfs->diff[i].fe[0],
           dfs->diff[i].code[0],dfs->diff[i].fs[1],dfs->diff[i].fe[1]);
    }
  }


diffs *rundiff(char *options,char *file0,char *file1)
  {
  static char *diff="diff %s %s %s";
  char buf[LINELEN];
  FILE *prog;
  int sfile[2];
  diffs *rv;
  int i,lines,width,w,di,j,n1,n0,t;
  char *endp,*p;
  
  /* open both files */
  sfile[0] = open(file0,O_RDONLY);
  if (sfile[0]<=0)
    {
    fprintf(stderr,"bxdiff: cant open %s\n",file0);
    exit(1);
    }

  /* file1 could be a dir or a file */
  struct stat stat1buf;
  if (stat(file1,&stat1buf))
    {
    close(sfile[0]);
    fprintf(stderr,"bxdiff: stat open %s\n",file1);
    exit(1);
    }

  /* find last slash */
  char *slash=file0;
  p=file0;
  while (*p)
    {
    if (*p == '/') slash=p;
    p++;
    }

  int f1len = strlen(file1) + (p-slash) + 4;
  char file1namebuf[f1len];
  
  if (S_ISDIR(stat1buf.st_mode))
    {
    strcpy(file1namebuf,file1);
    strcat(file1namebuf,slash);
    file1 = file1namebuf;
    }
  
  sfile[1] = open(file1,O_RDONLY);
  if (sfile[1]<=0)
    {
    close(sfile[0]);
    fprintf(stderr,"bxdiff: cant open %s\n",file1);
    exit(1);
    }

  rv =newitem(diffs,1);
  
  rv->file[0].name=file0;
  rv->file[1].name=file1;
  
  for (i=0;i<2;i++)
    { /* loop over both files */
    /* read in the file */
    rv->file[i].len = lseek(sfile[i],0,SEEK_END);
    lseek(sfile[i],0,SEEK_SET);
    rv->file[i].data = newitem(char,rv->file[i].len+1);
    read(sfile[i],rv->file[i].data,rv->file[i].len);
    rv->file[i].data[ rv->file[i].len ] = 0;
    close(sfile[i]);

    /* extract neato info from the file */
    lines = 0;
    width = 0;
    w=0;
    
    endp = rv->file[i].data + rv->file[i].len;
    
    for (p=rv->file[i].data ; p<endp ; p++)
      {
      if (w==0) lines++;
      if (*p=='\n')
        {
        if (w>width) width=w;
        w=0;
        }
      else
        {
        w++;
        }
      }

    rv->file[i].width = width;
    rv->file[i].lines = lines;

    /* make line array */
    w=0;
    lines=0;
    rv->file[i].line = newitem(char *,rv->file[i].lines);
    
    for (p=rv->file[i].data ; p<endp ; p++)
      {
      if (w==0) rv->file[i].line[lines++]=p;
      if (*p=='\n')
        {
        *p=0;
        w=0;
        }
      else
        {
        w++;
        }
      }
    }

  /* read in the diff */
  rv->diff = newitem(diffrec,min(rv->file[0].lines,rv->file[1].lines)+3);
  
  sprintf(buf,diff,options,file0,file1);
  prog = popen(buf,"r");
  di=0;
  
  for ( ; fgets(buf,LINELEN,prog) ; )
    if (buf[0]>='0' && buf[0]<='9')
      { /* must be a diff line */
      if (5==sscanf(buf,"%d,%d%1[acd]%d,%d",&rv->diff[di].fs[0],&rv->diff[di].fe[0],
                    rv->diff[di].code,&rv->diff[di].fs[1],&rv->diff[di].fe[1]))
        {
        rv->diff[di].fe[0]++;
        rv->diff[di].fe[1]++;
        }
      else if (4==sscanf(buf,"%d,%d%1[acd]%d",&rv->diff[di].fs[0],&rv->diff[di].fe[0],
                         rv->diff[di].code,&rv->diff[di].fs[1]))
        {
        rv->diff[di].fe[0]++;
        if (rv->diff[di].code[0]=='c')
          rv->diff[di].fe[1] = rv->diff[di].fs[1]+1;
        else
          rv->diff[di].fe[1] = ++(rv->diff[di].fs[1]);
        }
      else if (4==sscanf(buf,"%d%1[acd]%d,%d",&rv->diff[di].fs[0],
                         rv->diff[di].code,&rv->diff[di].fs[1],&rv->diff[di].fe[1]))
        {
        rv->diff[di].fe[1]++;
        if (rv->diff[di].code[0]=='c')
          rv->diff[di].fe[0] = rv->diff[di].fs[0]+1;
        else
          rv->diff[di].fe[0] = ++(rv->diff[di].fs[0]);
        }
      else if (3==sscanf(buf,"%d%1[acd]%d,%d",&rv->diff[di].fs[0],
                         rv->diff[di].code,&rv->diff[di].fs[1]))
        {
        if (rv->diff[di].code[0]=='c')
          {
          rv->diff[di].fe[1] = rv->diff[di].fs[1] + 1;
          rv->diff[di].fe[0] = rv->diff[di].fs[0] + 1;
          }
        else if (rv->diff[di].code[0]=='a')
          {
          rv->diff[di].fe[1] = rv->diff[di].fs[1] + 1;
          rv->diff[di].fe[0] = ++(rv->diff[di].fs[0]);
          }
        else if (rv->diff[di].code[0]=='d')
          {
          rv->diff[di].fe[0] = rv->diff[di].fs[0] + 1;
          rv->diff[di].fe[1] = ++(rv->diff[di].fs[1]);
          }
        }
      else
        {
        fprintf(stderr,"bxdiff: confusion...\n%s\n",buf);
        di--;
        }
      rv->diff[di].fs[0]--;
      rv->diff[di].fs[1]--;
      rv->diff[di].fe[0]--;
      rv->diff[di].fe[1]--;
      di++;
      }
  rv->diffs = di;

  rv->l01 = newitem(int,rv->file[0].lines+1);
  rv->l10 = newitem(int,rv->file[1].lines+1);

  for (i=1;i<rv->file[0].lines;i++) rv->l01[i]= -1;
  for (i=1;i<rv->file[1].lines;i++) rv->l10[i]= -1;

  rv->dr0 = newitem(int,rv->file[0].lines+1);
  rv->dr1 = newitem(int,rv->file[1].lines+1);

  for (i=1;i<rv->file[0].lines;i++) rv->dr0[i]= -1;
  for (i=1;i<rv->file[1].lines;i++) rv->dr1[i]= -1;
  

  for (i=0;i<di;i++)
    {
    n1 = rv->diff[i].fe[1]-rv->diff[i].fs[1];
    n0 = rv->diff[i].fe[0]-rv->diff[i].fs[0];
    if (n0>n1 && n0>0)
      {
      for (j=rv->diff[i].fs[0];j<rv->diff[i].fe[0];j++)
        {
        rv->l01[j] = t = (n1*(j-rv->diff[i].fs[0]))/n0 + rv->diff[i].fs[1];
        if (rv->l10[t]<0) rv->l10[t] = j;
        rv->dr0[j]=i;
        }
      }
    else if (n1>0)
      {
      for (j=rv->diff[i].fs[1];j<rv->diff[i].fe[1];j++)
        {
        rv->l10[j] = t = (n0*(j-rv->diff[i].fs[1]))/n1 + rv->diff[i].fs[0];
        if (rv->l01[t]<0) rv->l01[t] = j;
        rv->dr1[j]=i;
        }
      }
    rv->l01[rv->diff[i].fs[0]] = rv->diff[i].fs[1];
    rv->l01[rv->diff[i].fe[0]] = rv->diff[i].fe[1];
    rv->l10[rv->diff[i].fs[1]] = rv->diff[i].fs[0];
    rv->l10[rv->diff[i].fe[1]] = rv->diff[i].fe[0];
    
    for (j=rv->diff[i].fs[1];j<=rv->diff[i].fe[1];j++)
      rv->l10[j] = n1>0 ? (n0*(j-rv->diff[i].fs[1]))/n1 + rv->diff[i].fs[0]
                   : rv->diff[i].fe[0];
    }

  for (i=1;i<rv->file[0].lines;i++)
    if (rv->l01[i]<0) rv->l01[i] = rv->l01[i-1] +1;
  
  for (i=1;i<rv->file[1].lines;i++)
    if (rv->l10[i]<0) rv->l10[i] = rv->l10[i-1] +1;

  pclose(prog);
  return(rv);
  }
  
int main(int argn,char **argv,char **envp)
  {
  diffs *dfs;
  
  if (argn<3)
    {
    fprintf(stderr,"bxdiff: i need two files...\n");
    exit(1);
    }

  if (argn==3)
    dfs = rundiff("",argv[1],argv[2]);
  else
    dfs = rundiff(argv[1],argv[2],argv[3]);

  popitup(dfs);
  return 0;
  }

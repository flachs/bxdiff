#include "Xstuff.h"
#include "proto.h"
#include <sys/mman.h>

static inline char *cpystring(char *b,const char *p)
  {
  while ( *b++=*p++ );
  return b-1;
  }

static inline void draw_object(int expose,windowstuff *eventwind)
  {
  eventwind->draw(expose,eventwind);
  }


void popitup(diffs *dfs)
  {
  int r_x,r_y,p_x,p_y;
  unsigned int k_b;
  Window rwind,rchild;
  
  /* establish root connection */
  windowstuff *rw = openroot(NULL);

  if (!rw)
    {
    fprintf(stderr,"bxdiff: could not open display\n");
    exit(1);
    }

  /* figure out font stuff */
  XFontStruct *fontinfo = XLoadQueryFont(rw->display,"6x10");
  if (! fontinfo)
    {
    fprintf(stderr,"bxdiff: cant load font %s\n","6x10");
    exit(1);
    }
 
  /* create main window */
  if (dfs->file[0].width>MAXPANEWID) dfs->file[0].width=MAXPANEWID;
  if (dfs->file[1].width>MAXPANEWID) dfs->file[1].width=MAXPANEWID;

  int numlines = min(max(dfs->file[0].lines,dfs->file[1].lines),80);

  int panew[2],panex[2];
  panex[0] = BORDER;
  panew[0] = dfs->file[0].width*fontinfo->max_bounds.width+2*INDENT;
  panex[1] = panex[0]+panew[0]+XOVERWID;
  panew[1] = dfs->file[1].width*fontinfo->max_bounds.width+2*INDENT;

  int panelen = numlines*(fontinfo->ascent+fontinfo->descent);
  
  windowstuff *main = create_main(rw,
                          3*BORDER+panew[0]+panew[1]+XOVERWID+MAPWID,
                          2*BORDER+panelen+fontinfo->ascent+fontinfo->descent,
                          fontinfo,
                          dfs->file[0].name,dfs->file[1].name,
                          panex[0],panex[1]);

  /* create text panes */
  windowstuff *pane[2];
  for (int i=0;i<2;i++)
    {
    pane[i] = create_pane(main,panex[i],
                          fontinfo->ascent+fontinfo->descent+BORDER,
                          panew[i],panelen,i,dfs,fontinfo);
    }

  /* create map */
  windowstuff *map = create_map(main,panex[1]+panew[1]+BORDER,
                                fontinfo->ascent+fontinfo->descent+BORDER,
                                MAPWID,panelen,dfs);

  /* create xover */
  windowstuff *xover = create_xover(main,panex[0]+panew[0]+BORDER,
                                    fontinfo->ascent+fontinfo->descent+BORDER,
                                    panex[1]-BORDER-(panex[0]+panew[0]+BORDER),
                                    panelen,dfs,
                                    fontinfo->ascent+fontinfo->descent);

  /* finally the event loop */
  int reentrymode=0;
  int done;
  windowstuff *redia = 0;
  for (done=0; !done ; )
    {
    XNextEvent(main->display,&main->event);
    windowstuff *eventwind = find_win(main->event.xexpose.window) ;
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
      case KeyPress:;
        char text[100];
        KeySym  mykey;
        int nc = XLookupString((XKeyEvent *)&main->event, 
                               text, sizeof(text), &mykey, 0);
        if (nc==0)
          {// not a character key mykey=X11/keysymdef.h
          text[0]=0;
          if (0) printf("button %x\n",mykey);
          if (reentrymode)
            {
            }
          else
            {
            switch (mykey)
              {
              case XK_Right: /* find next match */
                if (redia)
                  {
                  while (movedown(map,1)) ;
                  motion_sync(map,xover,pane,numlines);
                  }
                break;
              case XK_Left: /* find pref match */
                if (redia)
                  {
                  while (moveup(map,1)) ;
                  motion_sync(map,xover,pane,numlines);
                  }
                break;
              case XK_Home:
                gototop(map);
                motion_sync(map,xover,pane,numlines);
                break;
              case XK_End:
                gotobot(map);
                motion_sync(map,xover,pane,numlines);
                break;
              case XK_Down:
                movedown(map,0);
                motion_sync(map,xover,pane,numlines);
                break;
              case XK_Up:
                moveup(map,0);
                motion_sync(map,xover,pane,numlines);
                break;
              case XK_Page_Up:
                for (int i=0;i<numlines/2;i++) moveup(map,0);
                motion_sync(map,xover,pane,numlines);
                break;
              case XK_Page_Down:
                for (int i=0;i<numlines/2;i++) movedown(map,0);
                motion_sync(map,xover,pane,numlines);
                break;
              }
            }
          }
        else
          {
          if (0) printf("nc %d %x\n",nc,text[0]);
          if (reentrymode)
            {
            switch (text[0])
              {
              case 0: break;
              case 0xd: // cr \r
                reentrymode=0;
                search_re(redia,map,xover,pane);
                break;
              case 0x8: // bs \b
                backspace_redia(redia);
                break;
              default:
                add_text_redia(redia,nc,text);
              }
            
            if (0) printf("key %x\n",text[0]);
            }
          else
            { // motion mode
            if (0) printf("KEY %x\n",text[0]);
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
                movedown(map,0);
                motion_sync(map,xover,pane,numlines);
                break;
              case ' ': /* page down */
                for (int i=0;i<numlines/2;i++) movedown(map,0);
                motion_sync(map,xover,pane,numlines);
                break;
              case 'k': /* up */
                moveup(map,0);
                motion_sync(map,xover,pane,numlines);
                break;
              case 'b': /* page up */
                for (int i=0;i<numlines/2;i++) moveup(map,0);
                motion_sync(map,xover,pane,numlines);
                break;
              case 'n': /* find next diff */
                while (movedown(map,0)) ;
                motion_sync(map,xover,pane,numlines);
                break;
              case 'p': /* find pref diff */
                while (moveup(map,0)) ;
                motion_sync(map,xover,pane,numlines);
                break;
              case '/': /* regexp find */
                reentrymode=1;
                if (redia)
                  reset_redia(redia);
                else
                  redia = create_redia(main,fontinfo,
                                       panex[0]+panew[0]/2,
                                       BORDER/2,
                                       panex[1]-BORDER-(panex[0]+panew[0]/2),
                                       fontinfo->ascent+fontinfo->descent,
                                       dfs);
                
                break;
              case 'N': /* find next match */
                if (redia)
                  {
                  while (movedown(map,1)) ;
                  motion_sync(map,xover,pane,numlines);
                  }
                break;
              case 'P': /* find pref match */
                if (redia)
                  {
                  while (moveup(map,1)) ;
                  motion_sync(map,xover,pane,numlines);
                  }
                break;
              }
            }
          }
      }
    }

  if (redia) destroy_redia(&redia);
  destroy_xover(&xover);
  destroy_map(&map);

  for (int i=0;i<2;i++) destroy_pane(&pane[i]);
  
  destroy_main(&main);
  XFreeFont(rw->display,fontinfo);
  closeroot(rw);
  }

void motion_sync(windowstuff *map,windowstuff *xover,
                 windowstuff **pane,int numlines)
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

int moveup(windowstuff *mapw,int resrch)
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

  if (resrch)
    {
    srch= -2;
    if (map->dif->file[0].matches[map->cur[0]] ||
        map->dif->file[1].matches[map->cur[1]] )
      return 0;
    return 1;
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

int movedown(windowstuff *mapw,int resrch)
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

  
  if (resrch)
    {
    srch= -2;
    if (map->dif->file[0].matches[map->cur[0]] ||
        map->dif->file[1].matches[map->cur[1]] )
      return 0;
    return 1;
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

void draw_marker(windowstuff *w,int x,int y,int dir)
  {
  XDrawLine(w->display,w->window,w->gc,
            x,y,x+dir*4,y);
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

char *rendertabs(char *in,int inl,char *out,int ml,int *rl)
  {
  int xc=0,tc=0;
  int i=0;
  char *p,*q;

  // are there any tabs?
  int l = min(inl,ml);
  for (p=in; p-in<l ; p++)
    if (*p=='\t') break;
  if (*p != '\t')
    {
    if (rl) *rl=inl;
    return in;
    }
  
  for (p=in,q=out; p-in<inl && i<ml ; p++)
    {
    if (*p=='\t')
      {
      tc++;
      do 
        {
        *q++ = ' ' ;
        i++;
        xc++;
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
  if (rl) *rl=q-out;
  return out;
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


char *mapfile(char *name,size_t *lenp,size_t *len,char *other)
  {
  struct stat s;
  if (stat(name,&s)) return 0;

  size_t len_name=0;
  if (S_ISDIR(s.st_mode))
    {
    if (! other) return 0;

    char *slash=strrchr(other,'/');
    if (slash) other = slash+1;
    len_name=strlen(name);
    if (name[len_name-1] != '/') name[len_name++] = '/';
    strcpy(name+len_name,other);
    len_name += strlen(other);
    if (stat(name,&s)) return 0;
    }
  else
    len_name=strlen(name);
  
  int fd = open(name,O_RDONLY);
  if (fd<=0) return 0;

  char *rv = mmap(NULL, s.st_size+2, PROT_READ, MAP_PRIVATE, fd, 0);
  close(fd);
  
  if (len) *len = s.st_size;
  if (lenp) *lenp = len_name;
  return rv;
  }

diffs *rundiff(char *options,char *file0n,char *file1dn)
  {
  diffs *rv =newitem(diffs,1);
  
  rv->file[0].data = mapfile(file0n,&rv->file[0].len_name,
                             &rv->file[0].len,NULL);
  rv->file[0].name = strdup(file0n);

  char file1n[strlen(file1dn)+strlen(file0n)+1];
  strcpy(file1n,file1dn);
  rv->file[1].data = mapfile(file1n,&rv->file[1].len_name,
                             &rv->file[1].len,NULL);
  rv->file[1].name = strdup(file1n);

  for (int i=0;i<2;i++)
    { /* loop over both files extract neato info from the file */
    int lines = 0;
    int width = 0;
    char *nl;
    
    char *endp = rv->file[i].data + rv->file[i].len;
    char *p=rv->file[i].data;
    while ((nl=strchr(p,'\n')) && nl<endp)
      {
      lines++;
      int w = nl - p;
      if (w>width) width=w;
      p = nl+1;
      }
    
    rv->file[i].width = width;
    rv->file[i].lines = lines;

    /* make line array */
    lines=0;
    rv->file[i].line = newitem(char *,rv->file[i].lines+1);
    rv->file[i].matches = newitem(int32_t,rv->file[i].lines);
    p=rv->file[i].data;
    while ((nl=strchr(p,'\n')) && nl<endp)
      {
      rv->file[i].line[lines++] = p;
      p = nl+1;
      }
    rv->file[i].line[lines] = endp;
    }

  size_t len_options = options ? strlen(options) : 0;
  char diff[4+3+1+len_options+rv->file[0].len_name+rv->file[1].len_name];
  char *p = cpystring(diff,"diff ");
  if (options) p=cpystring(cpystring(p,options)," ");
  cpystring(cpystring(cpystring(p,rv->file[0].name)," "),rv->file[1].name);

  FILE *prog= popen(diff,"r");
  
  /* read in the diff */
  int recs = 1024;
  rv->diff = newitem(diffrec,recs);
  int di=0;
  int linelen = max(rv->file[0].width,rv->file[1].width);
  char buf[linelen+5];
  
  while ( fgets(buf,linelen,prog) )
    if (buf[0]>='0' && buf[0]<='9')
      { /* must be a diff line */
      if (5==sscanf(buf,"%d,%d%1[acd]%d,%d",
                    &rv->diff[di].fs[0],&rv->diff[di].fe[0],
                    rv->diff[di].code,
                    &rv->diff[di].fs[1],&rv->diff[di].fe[1]))
        {
        rv->diff[di].fe[0]++;
        rv->diff[di].fe[1]++;
        }
      else if (4==sscanf(buf,"%d,%d%1[acd]%d",
                         &rv->diff[di].fs[0],&rv->diff[di].fe[0],
                         rv->diff[di].code,&rv->diff[di].fs[1]))
        {
        rv->diff[di].fe[0]++;
        if (rv->diff[di].code[0]=='c')
          rv->diff[di].fe[1] = rv->diff[di].fs[1]+1;
        else
          rv->diff[di].fe[1] = ++(rv->diff[di].fs[1]);
        }
      else if (4==sscanf(buf,"%d%1[acd]%d,%d",&rv->diff[di].fs[0],
                         rv->diff[di].code,
                         &rv->diff[di].fs[1],&rv->diff[di].fe[1]))
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
      if (di>=recs)
        {
        recs *= 2;
        rv->diff = realloc(rv->diff,recs*sizeof(diffrec));
        }
      }
  pclose(prog);
  
  rv->diffs = di;

  rv->l01 = newitem(int,rv->file[0].lines+1);
  rv->l10 = newitem(int,rv->file[1].lines+1);

  for (int i=1;i<rv->file[0].lines;i++) rv->l01[i]= -1;
  for (int i=1;i<rv->file[1].lines;i++) rv->l10[i]= -1;

  rv->dr0 = newitem(int,rv->file[0].lines+1);
  rv->dr1 = newitem(int,rv->file[1].lines+1);

  for (int i=1;i<rv->file[0].lines;i++) rv->dr0[i]= -1;
  for (int i=1;i<rv->file[1].lines;i++) rv->dr1[i]= -1;
  

  for (int i=0;i<di;i++)
    {
    int n1 = rv->diff[i].fe[1]-rv->diff[i].fs[1];
    int n0 = rv->diff[i].fe[0]-rv->diff[i].fs[0];
    if (n0>n1 && n0>0)
      {
      for (int j=rv->diff[i].fs[0];j<rv->diff[i].fe[0];j++)
        {
        int t = t = (n1*(j-rv->diff[i].fs[0]))/n0 + rv->diff[i].fs[1];
        rv->l01[j] = t;
        if (rv->l10[t]<0) rv->l10[t] = j;
        rv->dr0[j]=i;
        }
      }
    else if (n1>0)
      {
      for (int j=rv->diff[i].fs[1];j<rv->diff[i].fe[1];j++)
        {
        int t = (n0*(j-rv->diff[i].fs[1]))/n1 + rv->diff[i].fs[0];
        rv->l10[j] = t;
        if (rv->l01[t]<0) rv->l01[t] = j;
        rv->dr1[j]=i;
        }
      }
    rv->l01[rv->diff[i].fs[0]] = rv->diff[i].fs[1];
    rv->l01[rv->diff[i].fe[0]] = rv->diff[i].fe[1];
    rv->l10[rv->diff[i].fs[1]] = rv->diff[i].fs[0];
    rv->l10[rv->diff[i].fe[1]] = rv->diff[i].fe[0];
    
    for (int j=rv->diff[i].fs[1];j<=rv->diff[i].fe[1];j++)
      rv->l10[j] = n1>0 ? (n0*(j-rv->diff[i].fs[1]))/n1 + rv->diff[i].fs[0]
                   : rv->diff[i].fe[0];
    }

  for (int i=1;i<rv->file[0].lines;i++)
    if (rv->l01[i]<0) rv->l01[i] = rv->l01[i-1] +1;
  
  for (int i=1;i<rv->file[1].lines;i++)
    if (rv->l10[i]<0) rv->l10[i] = rv->l10[i-1] +1;

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

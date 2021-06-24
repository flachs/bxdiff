#include <ctype.h>

#include "Xstuff.h"
#include <ctype.h>

windowstuff *internalrootwindow=NULL;

windowstuff *openroot(char *name)
  {
  if (internalrootwindow==NULL)
    {
    internalrootwindow = newitem(windowstuff,1);
    if (!name) name="";
    internalrootwindow->display = XOpenDisplay(name);
    internalrootwindow->x11_fd  = ConnectionNumber(internalrootwindow->display);

    if (internalrootwindow->display)
      {
      internalrootwindow->screen = DefaultScreen(internalrootwindow->display);
      internalrootwindow->window = 
        DefaultRootWindow(internalrootwindow->display);
      internalrootwindow->depth = DefaultDepth(internalrootwindow->display,
                                               internalrootwindow->screen);
      internalrootwindow->vis = DefaultVisual(internalrootwindow->display,
                                              internalrootwindow->screen);
      internalrootwindow->cmap = DefaultColormap(internalrootwindow->display,
                                                 internalrootwindow->screen);
      }
    else
      {
      free(internalrootwindow);
      internalrootwindow = NULL;
      }
    }
  return(internalrootwindow);
  }

void closeroot()
  {
  if (internalrootwindow) 
    {
    XCloseDisplay(internalrootwindow->display);
    free(internalrootwindow);
    internalrootwindow=NULL;
    }
  }

windowstuff *openwindow_pm(windowstuff *root,char *name,int hints,
                        int x,int y,int w,int h,
                        int b,int s,int evmsk,Pixmap pm)
  {
  if (root==NULL) root = openroot("");

  windowstuff *rv = newitem(windowstuff,1);
  *rv = *root;

  /* default pixel values */
  if (b>0)
    {
    rv->background = BlackPixel(rv->display,rv->screen);
    rv->foreground = WhitePixel(rv->display,rv->screen);
    }
  else
    {
    rv->background = WhitePixel(rv->display,rv->screen);
    rv->foreground = BlackPixel(rv->display,rv->screen);
    }
  
  /* default program-specified window position and size */

  rv->hint.x = x;
  rv->hint.y = y;
  rv->hint.width  = w; 
  rv->hint.height = h;
  rv->hint.flags = hints;
  rv->attribs.backing_store = (s) ? Always : NotUseful;
  rv->attribs.background_pixel = rv->background ;
  rv->attribs.background_pixmap = pm;

  rv->window = XCreateWindow(rv->display,root->window,
        rv->hint.x, rv->hint.y, rv->hint.width, rv->hint.height,
        0,CopyFromParent,InputOutput,CopyFromParent,
        ( (s) ? CWBackingStore : 0 ) | 
          ( (b && pm==None) ? CWBackPixel : 0 ) | 
          ( (pm!=None) ? CWBackPixmap : 0) ,
        &rv->attribs);
  XSetStandardProperties(rv->display, rv->window,
        name , name ,
        None, NULL, 0, &rv->hint);

  /* GC creation and initialization */
  rv->gc = XCreateGC(rv->display, rv->window, 0, 0);
  XSetBackground(rv->display, rv->gc, rv->background);
  XSetForeground(rv->display, rv->gc, rv->foreground);

  /* input event selection */
  XSelectInput(rv->display, rv->window,evmsk);
  Atom wmDelete=XInternAtom(rv->display, "WM_DELETE_WINDOW", True);
  XSetWMProtocols(rv->display, rv->window, &wmDelete, 1);
  
  /* window mapping */
  XMapRaised(rv->display, rv->window);
  return(rv);
  }

windowstuff *openwindow(windowstuff *root,char *name,int hints,
                        int x,int y,int w,int h,
                        int b,int s,int evmsk)
  {
  return(openwindow_pm(root,name,hints,x,y,w,h,b,s,evmsk,None));
  }

void closewindow(windowstuff *rv)
  {
  XFreeGC(rv->display,rv->gc);
  XDestroyWindow(rv->display,rv->window);
  XFlush(internalrootwindow->display);
  free(rv);
  }

/* 
   gray scale support: perhaps slow but doesnt screw up other
   apps 
   uses 64 colors for good trade of between what is visable and
   small color translation table.
*/

unsigned long GrayScaleXTab[GRAY_LEVELS];

void setupgrayscalecolormap(windowstuff *w)
  {
  XColor retdef;
  int val,top,base;
  FILE *p;
  
  p = popen("gray-scale-range","r");
  if (p)
    {
    fscanf(p,"%d%d",&top,&base);
    pclose(p);
    }
  else
    {
    top=255;
    base=0;
    }
  
  for (val=0;val<GRAY_LEVELS;val++)
    {
    retdef.red = retdef.green = retdef.blue = (((top-base)*val/GRAY_LEVELS)+base)<<8;
    XAllocColor(w->display,w->cmap,&retdef);
    GrayScaleXTab[val] = retdef.pixel;
    }
  }

int isinited;

int proc(XrmDatabase *database,
         XrmBindingList bindings,
         XrmQuarkList quarks,
         XrmRepresentation *type,
         XrmValue *value,
         char *arg)
  {
  int qi =0;
  printf("here");
  while (quarks[qi]) printf(" %s",XrmQuarkToString(quarks[qi++]));
    
  printf(" : %s\n",value->addr);
  return(0);
  }

char *GRappname;

void ParseOptions(int *argc,char **argv,optionsdatabase *odb)
  {
  char str[80];
  char filename[1024];
  char *sptr;
  int i;
  static XrmDatabase home;
  static XrmDatabase commandline;
  extern char *getenv();

  if (!isinited) 
    {
    XrmInitialize();
    isinited=1;
    }

  /* Get command line options */
  if (!GRappname)
    {
    char *cmd = strrchr(argv[0],'/');
    GRappname = (cmd) ? cmd+1 : argv[0];
    }
  
  XrmParseCommand(&commandline,
                  odb->opTable,odb->opTableEntries,
                  GRappname,argc,argv);

  sptr = getenv("HOME");
  strncpy(filename,sptr,sizeof(filename));
  strncat(filename,"/.Xdefaults",sizeof(filename) - strlen(filename));
  filename[sizeof(filename)-1] = '\0';
  home = XrmGetFileDatabase(filename);
  XrmMergeDatabases(home,&(odb->res));
  
  XrmMergeDatabases(commandline,&(odb->res));
  
  if (0)
    {
    XrmQuarkList nql = calloc(sizeof(XrmQuark),10);
    XrmQuarkList cql = calloc(sizeof(XrmQuark),10);
    nql[0] = XrmStringToQuark("xlauncher");
    cql[0] = XrmStringToQuark("Xlauncher");
  
    XrmEnumerateDatabase(odb->res, nql, cql,
                         XrmEnumAllLevels
                         // XrmEnumOneLevel
                         , proc, "ok");
    }
  
  
  
  }

void PutResource(optionsdatabase *odb,char *name,char *value)
  {
  char dname[256];
  
  if (!isinited) 
    {
    XrmInitialize();
    isinited=1;
    }
  
  if (GRappname)
    {
    strcpy(dname,GRappname);
    strcat(dname,name);
    name = dname;
    }
      
  XrmPutStringResource(&(odb->resput),name,value);
  }

void PutMainResource(optionsdatabase *odb,char *name,char *value)
  {
  char dname[256];
  
  if (!isinited) 
    {
    XrmInitialize();
    isinited=1;
    }
  
  if (GRappname)
    {
    strcpy(dname,GRappname);
    strcat(dname,name);
    name = dname;
    }
      
  XrmPutStringResource(&(odb->res),name,value);
  }

                 
char *GetResource(optionsdatabase *odb,char *name,char *class)
  {
  XrmValue value;
  char *str_type[40];
  static char r[256];
  int s;
  char *p,*q,dclass[256],dname[256];

  if (GRappname)
    {
    strcpy(dname,GRappname);
    strcat(dname,name);
    name = dname;
    }
      
  if (!class)
    {
    for (p=name,q=dclass,s=0; *p ; p++, q++)
      {
      if (s)
        {
        *q = toupper(*p);
        s=0;
        }
      else
        {
        *q = *p;
        if (*p == '.') s=1;
        }
      }
    *q=0;
    class = dclass;
    }

  if(XrmGetResource(odb->res, name, class, str_type, &value) == True)
    {
    strncpy(r,value.addr,(int) value.size);
    r[(int) value.size+1]=0;
    return(r);
    }
  else if(XrmGetResource(odb->resput, name, class, str_type, &value) == True)
    {
    strncpy(r,value.addr,(int) value.size);
    r[(int) value.size+1]=0;
    return(r);
    }
  else 
    return(NULL);
  }


int GetGeometry(optionsdatabase *odb,char *name,char *class,
                 windowstuff *r,int s,int *x,int *y,int *w,int *h)
  {
  char *b;
  int f,X,Y,hf;
  unsigned int W,H;
  
  hf =0;

  b = GetResource(odb,name,class);

  if (b)
    {
    f=XParseGeometry(b,&X,&Y,&W,&H);

    if ( (f & WidthValue) && (s & WidthValue))
      {
      hf |= USSize;
      *w = W;
      }
    if ( (f & HeightValue) && (s & HeightValue))
      {
      hf |= USSize;
      *h = H;
      }

    if ( (f & XValue) && (s & XValue) )
      {
      hf |= USPosition;
      if (f & XNegative)
        *x = DisplayWidth(r->display,r->screen) + X - *w;
      else
        *x = X;
      }
    if ( (f & YValue) && (s & YValue))
      {
      hf |= USPosition;
      if (f & YNegative)
        *y = DisplayHeight(r->display,r->screen) + Y - *h;
      else
        *y = Y;
      }
    return(hf);
    }
  else
    return(PPosition | PSize);
  }



#define HASH_LEN 256
int Xrout_ass_debug=0;

int window_hash(Window w)
  {
  int r;
  r = (w>>1) & 0xff;
  if (Xrout_ass_debug) printf("hash: w %x h %x\n",w,r);
  return( r );  
  }

typedef struct Xrout_link_s
  {
  struct Xrout_link_s *next;
  Window w;
  windowstuff *m;
  } Xrout_link;

Xrout_link *Xrout_hashtab[HASH_LEN];

void dumphash()
  {
  Xrout_link **p,**r;
  int i;

  printf("associative hash dump\n");
  for (i=0;i<HASH_LEN;i++) if (Xrout_hashtab[i])
    {
    printf("hash %d\n",i);
    r = p = &Xrout_hashtab[i] ;
    for ( ; *p ; p = &((*p)->next) )
      {
      printf("  p %p *p %p (*p)->w %x (*p)->m %p (*p)->p %x\n",
        p,*p,(*p)->w,(*p)->m,(*p)->next);
      }
    }
  }
     
void disassociate(Window w)
  {
  int h;
  Xrout_link **p,*q;

  h=window_hash(w);
  p = &Xrout_hashtab[h] ;

  if (Xrout_ass_debug) printf("disassociate: w %x h %x p %x\n",w,h,p);

  for ( ; *p ; p = &((*p)->next) )
    {
    if (Xrout_ass_debug) printf("dis: p %p *p %p *pw %x *pm %p *pn %p\n",
       p,*p,(*p)->w,(*p)->m,(*p)->next);
    if ( (*p)->w==w ) 
      {
      q = (*p)->next;      
      free(*p);
      *p = q;
      if (Xrout_ass_debug&2) dumphash();
      return;
      }
    }
  if (Xrout_ass_debug&2) dumphash();
  printf("disassociate: not found %x\n",w);
  }

windowstuff *find_associate(Window w) 
  {
  Xrout_link **p,**r;
  int h;

  if (Xrout_ass_debug&2) dumphash();
  h = window_hash(w);
  r = p = &Xrout_hashtab[h] ;
  if (Xrout_ass_debug) printf("find_associate: w %x h %x r %p p %p\n",w,h,r,p);
  for ( ; *p ; p = &((*p)->next) )
    {
    if (Xrout_ass_debug) printf("fass: p %p *p %p *pw %x *pm %p *pn %p\n",
     p,*p,(*p)->w,(*p)->m,(*p)->next);
    if ( (*p)->w==w )
      {
      return( (*p)->m );
      }
    }
  return(0);
  }

void associate(Window w,windowstuff *m)
  {
  Xrout_link **p,*r;
  windowstuff *t;
  int h;

  h = window_hash(w);
  p = &Xrout_hashtab[h] ;
  r = *p;
  *p = newitem(Xrout_link,1);

  if (Xrout_ass_debug) printf("associate: w %x m %p h %x r %p p %p *p %p\n",w,m,h,r,p,*p);

  (*p)->w = w;
  (*p)->m = m;
  (*p)->next = r;

  t = find_associate(w);
  if (t!=m)
    {
    printf("ass: w %x h %x m %p p %p *p %p r %p\n",
           w,h,m,p,*p,r); 
    Xrout_ass_debug=3;
    t = find_associate(w);
    exit(1);
    }
  if (Xrout_ass_debug&2) dumphash();
  }


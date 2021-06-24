#include "common.h"

/* X include files */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/X.h>
#include <X11/Xresource.h>


#define FALSE   0
#define LEFT    1
#define MIDDLE  2
#define RIGHT   3

#define GRAY_LEVELS 64

typedef struct
  {
  char DisplayName[256];
  Display *display;
  int x11_fd;
  Colormap cmap;
  Visual *vis;
  int depth;
  int screen;
  struct local_struct *local;
  int background,foreground;
  Window  window;
  GC gc;
  XEvent event;
  KeySym  key;
  XSizeHints hint;
  XSetWindowAttributes attribs;
  } windowstuff;

/* include typedef local_type */
#include <Xlocal.h>

typedef struct 
  {
  char *name;
  int opTableEntries;
  XrmOptionDescRec *opTable;
  XrmDatabase res;
  XrmDatabase resput;
  } optionsdatabase ;

/* function prototypes */
windowstuff *openroot(char *name);
void closeroot();
windowstuff *openwindow(windowstuff *root,char *name,int hints,
                        int x,int y,int w,int h,
                        int b,int s,int evmsk);
windowstuff *openwindow_pm(windowstuff *root,char *name,int hints,
                        int x,int y,int w,int h,
                        int b,int s,int evmsk,Pixmap pm);
void closewindow(windowstuff *rv);
void ParseOptions(int *argc,char **argv,optionsdatabase *odb);

char *GetResource(optionsdatabase *odb,char *name,char *class);
void PutResource(optionsdatabase *odb,char *name,char *value);

int GetGeometry(optionsdatabase *odb,char *name,char *class,
                 windowstuff *r,int s,int *x,int *y,int *w,int *h);
void associate(Window w,windowstuff *m);
void disassociate(Window w);
windowstuff *find_associate(Window w);
void setupgrayscalecolormap(windowstuff *w);

/* global structures */
extern unsigned long GrayScaleXTab[GRAY_LEVELS];
extern char *GRappname;

/* very common routines */
#define xline(w,x1,y1,x2,y2) XDrawLine((w)->display,(w)->window,(w)->gc,(int)(x1),(int)(y1),(int)(x2),(int)(y2))
#define pencolor(w,c) XSetForeground((w)->display,(w)->gc,( (c) ? (w)->foreground : (w)->background))
#define pengray(w,c) XSetForeground((w)->display,(w)->gc,GrayScaleXTab[c])
#define xstring(w,x,y,s,l) XDrawString((w)->display,(w)->window,(w)->gc,(int)(x),(int)(y),s,(int)(l))
#define xflush(w) XFlush((w)->display)
#define xclear(w) XClearWindow((w)->display,(w)->window)
#define xarc(w,x,y,a,b,c,d) XDrawArc((w)->display,(w)->window,(w)->gc,x,y,a,b,c,d)
#define xpoint(w,x,y) XDrawPoint((w)->display,(w)->window,(w)->gc,(int)(x),(int)(y))
#define assoc_win(w) associate((w)->window,(w))
#define find_win(w)  find_associate(w)
#define disassoc_win(w) disassociate((w)->window)

#define LINELEN 1000
#define MAXPANEWID 80
#define XOVERWID 80
#define MAPWID 50
#define MAXHGT 80
#define TITLEHGT 25
#define BUTTONHGT 25

#define BORDER 5
#define INDENT 3
#define PCRE2_CODE_UNIT_WIDTH 8

#include <pcre2.h>

#define FunnyMode GXxor
#define xistring(w,x,y,s,l) XDrawImageString((w)->display,(w)->window,(w)->gc,(int)(x),(int)(y),s,(int)(l))

typedef struct
  {
  char *name;
  size_t len_name;
  size_t len;
  int lines;
  int width;
  int32_t *matches;  // 0=none >0=one(start<<16|end) <0=-(num<<16|index)
  char *data;
  char **line;
  } fileinfo;

typedef struct
  {
  int fs[2],fe[2];
  char code[3];
  } diffrec;

typedef struct
  {
  fileinfo file[2];
  int diffs;
  diffrec *diff;
  int *l01,*l10;
  int *dr0,*dr1;
  } diffs;

typedef enum 
  {
  ROOT, MAIN, BUTTON, PANE, MAP, XOVER, REDIA
  } objects ;

typedef enum
  {
  EXIT, UP, DOWN, NEXT, PREV, PGDN, PGUP, buttonLAST
  } button_types;

typedef struct
  {
  windowstuff *w;
  char *name[2];
  int panex[2];
  XFontStruct *fontinfo;
  } main_desc;
  
typedef struct
  {
  windowstuff *w;
  button_types id;
  char *label;
  } button_desc;

typedef struct
  {
  windowstuff *w;
  XFontStruct *fontinfo;
  int side;
  diffs *dif;
  int32_t *matches;
  int startline;
  int pstartline;
  } pane_desc;

typedef struct
  {
  windowstuff *w;
  diffs *dif;
  int start[2];
  int cur[2];
  int h;
  } xover_desc;

typedef struct
  {
  windowstuff *w;
  XFontStruct *fontinfo;
  size_t len_re,size_re;
  char *re;
  diffs *dif;
  int32_t *matches;
  size_t size_matches;
  int start[2];
  int cur[2];
  } redia_desc;

typedef struct
  {
  windowstuff *w;
  diffs *dif;
  int cur[2];
  int pcur[2];
  } map_desc;


typedef struct local_struct 
  {
  objects object;
  union 
    {
    main_desc *main;
    button_desc *button;
    pane_desc *pane;
    xover_desc *xover;
    map_desc *map;
    redia_desc *redia;
    } desc;
  } local_type;

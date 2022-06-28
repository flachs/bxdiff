#define LINELEN 1000
#define MAXPANEWID 80
#define XOVERWID 80
#define MAPWID 50
#define MAXHGT 80
#define TITLEHGT 25
#define BUTTONHGT 25

#define BORDER 5
#define INDENT 3

typedef struct
  {
  char *name;
  int lines;
  int len;
  int width;
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
  ROOT, MAIN, BUTTON, PANE, MAP, XOVER
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
    } desc;
  } local_type;

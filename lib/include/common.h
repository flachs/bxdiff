#ifndef COMMON
/* don't include twice */
#define COMMON 1

/* very useful unix includes */
#include "stdio.h"
#include "unistd.h"
#include "fcntl.h"
#include "math.h"
#include "values.h"
#include "string.h"
#include "malloc.h"
#include "sys/types.h"
#include "signal.h"
#include "sys/stat.h"
#include "sys/time.h"
#include "sys/wait.h"
#include <stdlib.h>

/* some function definitions left out of the system include files */
#ifdef _HPUX_SOURCE 
#define rint(x) (floor((x)+.5))
#else
double rint();
#endif

/* compressed file open and close */
typedef FILE ZFILE;
ZFILE *Zopen(char *name,char *mode);
void Zclose(ZFILE *r);

/* unix/dos fopen binary */
#ifdef unix
#define RB "r"
#define WB "w"
#else
#define RB "rb"
#define WB "wb"
#endif

/* some useful constants */
#ifndef PI
#define PI M_PI
#endif

#define SQRT2PI 2.50662827463100050241

#include "space_shift.h"

/* some useful macros */
#define dumpcore { kill(getpid(),SIGQUIT); while (1) ; }
#define strdiff(a,b) strcmp((a),(b))
#define strndiff(a,b,n) strncmp((a),(b),(n))
#define sqr(a) ( (a)*(a) )
#define abs(a) ( (a) >= 0 ? (a) : -(a) )
#define sgn(a) ( (a) > 0 ? 1 : ( (a) < 0 ? -1 : 0) )
#define min(a,b) ( (a) > (b) ? (b) : (a) )
#define max(a,b) ( (a) > (b) ? (a) : (b) )
#define downshift(m,n) ( ((n)>0) ? (((m)>>(n))&( ~((unsigned int)1<<(unsigned int)(8*sizeof(m)-1)) )) : (m) )
#define newitem(i,n) (i *)calloc(n,sizeof(i))
#define newitem_ni(i,n) (i *)malloc((n)*sizeof(i))


#define RVNFL(d,s,c,e,i,u)                                                   \
  {                                                                          \
  int l=0,f=0;                                                               \
  while (!f) {                                                               \
    if (l<(d)) { (c)[l]=(s)[l]; l++; continue; }                             \
    u ;                                                                      \
    while (! ((c)[l-1]+(i)[l-1]<(e)[l-1]) ) { if (l>1) l--; else { f=1; break; } } \
    (c)[l-1]+=(i)[l-1];                                                      \
    }                                                                        \
  }

#define SRVNFL(d,s,c,e,i,u)                                                  \
  {                                                                          \
  int l=0,f=0;                                                               \
  while (!f) {                                                               \
    if (l<(d)) { (c)[l]=(s); l++; continue; }                                \
    u ;                                                                      \
    while (! ((c)[l-1]+(i)<(e)) ) { if (l>1) l--; else { f=1; break; } }     \
    (c)[l-1]+=(i);                                                           \
    }                                                                        \
  }

#define IFASSGN(d,c,v) { if ((d) c (v)) d = v; }
#endif


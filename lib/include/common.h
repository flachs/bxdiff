#ifndef COMMON
/* don't include twice */
#define COMMON 1

/* very useful unix includes */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <values.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdint.h>

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
#define abs(a) ({ typeof(a)_a=a; (_a >= 0) ? _a : -_a; })
#define sgn(a) ({ typeof(a)_a=a; (_a > 0)  ?  1 : (_a < 0) ? -1 : 0; })
#define min(a,b) ({ typeof(a)_a=a; typeof(b)_b=b; (_a > _b) ? _b : _a; })
#define max(a,b) ({ typeof(a)_a=a; typeof(b)_b=b; (_a > _b) ? _a : _b; })
#define newitem(i,n) (i *)calloc(n,sizeof(i))
#define newitem_ni(i,n) (i *)malloc((n)*sizeof(i))

// defense against >> weirdness
#define downshift(m,n) ({ typeof(m)_m=m,_o=1,_s=8*sizeof(_m)-1;typeof(n)_n=n;\
     (_n>0) ? ((_m>>_n)& ~(_o<<_s) ) : _m })

// multi-level nested for loops
// d - number of dimensions
// s - starting value (array for RVNFL constant for SRVNFL)
// c - current index array
// e - ending value or array (while (c<e) ....)
// i - increment value or array
// u - code to be looped
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


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>

#include "pstring.h"
#include "space_shift.h"

typedef enum 
  {
  PSTRING_SET,
  PSTRING_ADD
  } pstring_expand_mode_t;

static pstring_t *pstring_freed_head[32];

#define PICKHEAD(x) (ilog2(x+sizeof(pstring_t))+1)
#define CAPACITY(h) ((1<<h)-sizeof(pstring_t))

static pstring_t *pstring_new(size_t size)
  {
  pstring_t *rv;

  int h = SPACE_SHIFT(sizeof(pstring_t*)+size);
  
  if (rv = pstring_freed_head[h])
    {
    pstring_freed_head[h] = (pstring_t *)(rv->l);
    rv->l = 0;
    rv->t[0] = 0;
    return rv;
    }

  pstring_t *p = (pstring_t *)malloc(1<<h);
  p->m[0] = 0xff;
  p->m[1] = 'p';
  p->m[2] = 0;
  p->h = h;
  p->l = 0;
  p->t[0]=0;
  return p;
  }

void pstring_free(pstring_t **psp)
  {
  if (! psp)  return;
  if (! *psp) return;

  pstring_t *p = *psp;
  int h = p->h;
  p->l = (intptr_t)pstring_freed_head[h];
  pstring_freed_head[h] = p;
  *psp=NULL;
  }

static pstring_t *pstring_expand(pstring_t **psp,
                          pstring_expand_mode_t m,
                          size_t size)
  {
  pstring_t *p;
  
  if (size==0)     return(psp ? *psp : NULL);
  if (! psp)       return(pstring_new(size));
  if (! *psp)      return(*psp = pstring_new(size));
  
  p = *psp;

  if (m==PSTRING_ADD) size+=p->l;
  size++;

  if (size>CAPACITY(p->h))
    {
    pstring_t *q = pstring_new(size);
    size_t l = q->l = p->l;
    memcpy(q->t,p->t,l+1);
    pstring_free(psp);
    *psp = p = q;
    }
  return p;
  }

pstring_t *pstring_clear(pstring_t **psp)
  {
  if (! psp) return 0;
  if (! *psp) return 0;
  pstring_t *p = *psp;
  if (p->l)
    {
    p->l = 0;
    p->t[0] = 0;
    }
  return p;
  }

size_t pstring_len(const void *v)
  {
  pstring_t *p = (pstring_t *)v;
  if (!v) return 0;
  if (is_pstring(v)) return p->l;
  return strlen((char *)v);
  }

pstring_t *pstring_append(pstring_t **psp,const void *v)
  {
  size_t lv = pstring_len(v);
  pstring_t *p = pstring_expand(psp,PSTRING_ADD,lv);
  if (! lv) return p;

  char *t = p->t;
  unsigned long long l=p->l;
  
  memcpy(t+l, pstring_text(v), lv);
  
  p->l = l = l + lv;
  
  t[l] = 0;
  return p;
  }


pstring_t *pstring_append_c(pstring_t **psp,int it)
  {
  pstring_t *p = pstring_expand(psp,PSTRING_ADD,1);
  char *t = p->t;
  unsigned long long l=p->l;
  t[l++] = it;
  t[l] = 0;
  p->l = l;
  
  return p;
  }

pstring_t *pstring_append_int(pstring_t **psp,char *fmt,int it)
  {
  pstring_t *p = pstring_expand(psp,PSTRING_ADD,128);
  p->l += sprintf(p->t+p->l,fmt,it);
  return p;
  }
                
pstring_t *pstring_append_dbl(pstring_t **psp,const char *fmt,double it)
  {
  pstring_t *p = pstring_expand(psp,PSTRING_ADD,128);
  p->l += sprintf(p->t+p->l,fmt,it);
  
  return p;
  }

pstring_t *pstring_append_str(pstring_t **psp,const char *fmt,void *it)
  {
  int minwidth = strtol(fmt,NULL,10);
  int len = pstring_len(it);

  if (minwidth<0) minwidth = -minwidth;
  
  int ex = (minwidth>len) ? minwidth : len;
  
  pstring_t *p = pstring_expand(psp,PSTRING_ADD,ex+1);
  p->l += sprintf(p->t+p->l,fmt,it);
  
  return p;
  }

pstring_t *pstring_append_llint(pstring_t **psp,const char *fmt,
                                long long int it)
  {
  pstring_t *p = pstring_expand(psp,PSTRING_ADD,128);
  p->l += sprintf(p->t+p->l,fmt,it);
  
  return p;
  }

static const char *findendofplaceholder(const char *f,int *tcode)
  {
  int x = *f++;
  if (x=='%') return f;

  // flags
  if (x=='-' ||
      x=='+' ||
      x==' ' ||
      x=='0' ||
      x=='#' ) 
    x = *f++;
  
  if (x=='*') abort(); // dont handle variable width
  
  if (x>'0' && x<=9)
    { // min width
    strtol(f-1,(char **)&f,10);
    x = *f++;
    }
  
  if (x=='.')
    { // precision
    strtol(f,(char **)&f,10);
    x = *f++;
    }
  
  // length
  int what = 0;
  if (x=='h')
    {
    what = 'i';
    if (f[0] == x)
      {
      f++;
      }
    }
  else if (x=='l')
    {
    what = 'i';
    if (f[0] == x)
      {
      f++;
      what = 'l';
      }
    }
  else if (x=='l')
    {
    what = 'i';
    }
  else if (x=='L')
    {
    what = x;
    }
  else if (x=='z' || x=='j' || x=='t')
    {
    what = 'l';
    }
  if (x=='d' || x=='i' || x=='u' ||
      x=='x' || x=='X' || x=='o' ||
      x=='c')
    {
    if (what != 'l') what = 'i';
    }
  else if (x=='p')
    {
    what = 'l';
    }
  else if (x=='e' || x=='E' ||
           x=='f' || x=='F' ||
           x=='g' || x=='G' )
    {
    what = 'g';
    }
  else if (x=='s')
    {
    what = 's';
    }

  if (what) *tcode = what;
  return f;
  }


pstring_t *pstring_fappend(pstring_t **psp,const char *fmt, ...)
  {
  va_list ap;
  va_start(ap,fmt);

  int c;
  const char *f;
  for (f=fmt; c=*f++ ; )
    {
    if (c == '%')
      {
      const char *ph=f-1;
      int tcode = 0;
      
      f = findendofplaceholder(f,&tcode);

      if (! tcode)
        {
        for (const char *p=ph; p<f ; p++)
          pstring_append_c(psp,*p);
        continue;
        }

      int phl = f-ph;
      char phs[phl+1];
      strncpy(phs,ph,phl);
      phs[phl]=0;
      
      switch (tcode)
        {
        case 'i': pstring_append_int(psp,phs,va_arg(ap,int));    break;
        case 'l': pstring_append_llint(psp,phs,va_arg(ap,long long int));    break;
        case 'g': pstring_append_dbl(psp,phs,va_arg(ap,double)); break;
        case 's': pstring_append_str(psp,phs,va_arg(ap,void *)); break;
        default:
          abort();
        }
      }
    else pstring_append_c(psp,c);
    }
  va_end(ap);
  return *psp;
  }


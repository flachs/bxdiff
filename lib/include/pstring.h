// pascal-like string package supporting auto-space allocation.
// also supports recycling.

#ifndef __PSTRING_H__
#define __PSTRING_H__

typedef struct pstring_s
  {
  unsigned char m[7];
  int8_t h;
  intptr_t l;
  char t[0];
  } pstring_t;

static inline int is_pstring(const void *v)
  {
  if (!v) return 0;
  pstring_t *p = (pstring_t *)v;
  if (p->m[0]==0xff && p->m[1]=='p') return 1;
  return 0;
  }

static inline char *pstring_text(const void *v)
  {
  pstring_t *p = (pstring_t *)v;
  return v ? ((is_pstring(v)) ? p->t : (char *)v) : NULL;
  }

#ifdef __cplusplus
extern "C" 
{
#endif
void pstring_free(pstring_t **psp);
size_t pstring_len(const void *v);
pstring_t *pstring_clear(pstring_t **psp);

pstring_t *pstring_append_c(pstring_t **psp,int it);
pstring_t *pstring_append_int(pstring_t **psp,char *fmt,int it);
pstring_t *pstring_append_dbl(pstring_t **psp,const char *fmt,double it);
pstring_t *pstring_append_str(pstring_t **psp,const char *fmt,void *it);
pstring_t *pstring_append_llint(pstring_t **psp,const char *fmt,
                                long long int it);


pstring_t *pstring_append(pstring_t **psp,const void *v);
pstring_t *pstring_fappend(pstring_t **psp,const char *fmt, ...);

#ifdef __cplusplus
}
#endif


#endif

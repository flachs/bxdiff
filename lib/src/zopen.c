#include <unistd.h>

#include "common.h"

#define BUFLEN 1000

ZFILE *Zopen(char *name,char *mode)
  {
  ZFILE *r;
  FILE *f;
  char buf[BUFLEN];

  if (!strdiff(mode,"r") || !strdiff(mode,"rb"))
    {
    sprintf(buf,"/bin/gzip -cd %s",name);
    f = fopen(name,mode);
    if (f)
      {
      fclose(f);
      r = popen(buf,mode);
      return(r);
      }
    else
      return(NULL); /* can't open file */
    }
  else if (!strdiff(mode,"w") || !strdiff(mode,"wb"))
    {
    sprintf(buf,"/bin/gzip > %s",name);
    f = fopen(name,mode);
    if (f)
      {
      fclose(f);
      unlink(name);
      f = popen(buf,mode);
      return(r);
      }
    else
      return(NULL);  /* can't open file */
    }
  else
    return(NULL);    /* invalid mode */
  }


void Zclose(ZFILE *r)
  {
  pclose(r);
  }

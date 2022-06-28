#include "Xstuff.h"
#include "proto.h"

windowstuff *create_redia(windowstuff *main,XFontStruct *fontinfo,
                          int x,int y,int w,int l,
                          diffs *dfs)
  {
  windowstuff *rv;
  
  rv = openwindow(main,"Regexp",PPosition,x,y,w,l,-1,0,
                  ExposureMask|StructureNotifyMask);
  XSetFont(rv->display,rv->gc,fontinfo->fid);
  assoc_win(rv);
  rv->local = newitem(local_type,1);
  rv->local->object = REDIA;
  rv->local->desc.redia = newitem(redia_desc,1);
  rv->local->desc.redia->w = rv;
  rv->local->desc.redia->fontinfo = fontinfo;
  rv->local->desc.redia->len_re = 0;
  rv->local->desc.redia->size_re = 128;
  rv->local->desc.redia->re = malloc(rv->local->desc.redia->size_re);
  rv->local->desc.redia->dif = dfs;
  rv->local->desc.redia->start[0]=0;
  rv->local->desc.redia->start[1]=0;
  rv->local->desc.redia->cur[0]=0;
  rv->local->desc.redia->cur[1]=0;  
  draw_redia(0,rv->local->desc.redia);
  return(rv);
  }

void add_text_redia(windowstuff *redia,int nc,char *text)
  {
  redia_desc *rd = redia->local->desc.redia;
  size_t len = rd->len_re + nc;
  text[nc]=0;

  while (len > rd->size_re)
    {
    rd->size_re *= 2;
    rd->re = realloc(rd->re,rd->size_re);
    }
  
  strcpy(rd->re+rd->len_re,text);
  rd->len_re = len;
  draw_redia(0,rd);
  }

void backspace_redia(windowstuff *redia)
  {
  redia_desc *rd = redia->local->desc.redia;
  if (rd->len_re==0) return;
  rd->len_re--;
  rd->re[rd->len_re]=0;
  draw_redia(0,rd);
  }

void reset_redia(windowstuff *redia)
  {
  redia_desc *rd = redia->local->desc.redia;

  rd->len_re=0;
  rd->re[rd->len_re]=0;
  draw_redia(0,rd);
  }

int get_num_matches(int32_t nind)
  {
  if (nind==0) return 0;
  if (nind>0 ) return 1;
  nind = -nind;
  return nind>>16;
  }

int32_t get_match(int32_t nind,int32_t matches[],int m)
  {
  if (nind==0) return 0; // empty list
  if (nind>0 ) return nind;  // one element return it

  // two or more in list
  nind = -nind;
  int n=nind>>16;
  int ind=nind&0xffff;
  int32_t sti = matches[n];
  int s = sti>>16;
  int ti = sti & 0xffff;
  int32_t rv = matches[ti+ind+m];
  if (0) printf("GM: %d,%d -> %u -> %u %u\n",n,ind,ti,rv>>16,rv&0xffff);
  return rv;
  }

int32_t find_match(int num_matches,int32_t *matches,redia_desc *rd)
  {
  if (!rd->matches) return 0; // no match table yet
  
  int32_t *mt = rd->matches;
  int32_t nind = mt[num_matches];
  if (! nind) return 0;
  int n = nind>>16;
  int ind = nind&0xffff;
  int32_t *mp = mt+ind, *omp=mp;
  int32_t *mmp;
  for (int i=0;i<n;i++)
    {
    int found=1;
    mmp=mp;
    for (int m=0;m<num_matches;m++)
      if (matches[m]!=*mp++) found=0;
    if (found) return -((num_matches<<16)|(mmp-omp));
    }
  return 0;
  }

int32_t find_or_make_match(int num_matches,int32_t *matches,redia_desc *rd)
  {
  if (num_matches==0) return 0;
  if (num_matches==1) return matches[0];
  
  int32_t rv = find_match(num_matches,matches,rd);
  if (rv) return rv;

  int32_t *mt = rd->matches;
  if (!mt)
    { // no match table yet
    mt = rd->matches = newitem(int32_t,rd->size_matches = 1024);
    }
  
  int32_t nind = mt[num_matches];
  int n = nind>>16;
  int ind = nind&0xffff;
  
  if (! nind)
    { // new num_matches catagory
    ind = mt[0]; // first unused table entry
    n = 128;
    mt[0] += n*num_matches;
    while (mt[0] > rd->size_matches)
      { // table not big enough -- grow it
      size_t new_size = rd->size_matches + 1024;
      mt = rd->matches = realloc(mt,new_size*sizeof(mt[0]));
      memset(mt+rd->size_matches,0,(new_size-rd->size_matches)*sizeof(mt[0]));
      rd->size_matches = new_size;
      }
    mt[num_matches] = (n<<16)|ind;
    }

  // search for blank entry
  int32_t *mp = mt+ind, *omp=mp;
  int32_t *mmp;
  for (int i=0;i<n;i++)
    {
    int found=1;
    mmp=mp;
    for (int m=0;m<num_matches;m++)
      if (*mp++!=0) found=0;
    if (found)
      {
      memcpy(mmp,matches,num_matches*sizeof(matches[0]));
      return -((num_matches<<16)|(mmp-omp));
      }
    }

  // no blank - entries => expand table
  int exp = 128*num_matches;
  while (mt[0]+exp > rd->size_matches)
    { // table not big enough -- grow it
    size_t new_size = rd->size_matches + 1024;
    mt = rd->matches = realloc(mt,new_size*sizeof(mt[0]));
    memset(mt+rd->size_matches,0,(new_size-rd->size_matches)*sizeof(mt[0]));
    rd->size_matches = new_size;
    }

  int after = ind+n;
  memmove(mt+after+exp,mt+after,sizeof(mt[0])*(mt[0]-after));
  memset(mt+after,0,exp*sizeof(mt[0]));
  for (int i=2;i<16;i++)
    {
    int32_t enind = mt[i];
    int en = nind>>16;
    int eind = nind&0xffff;
    if (eind>=after)
      mt[i] = (en<<16)|(eind+exp);
    }
  
  memcpy(mt+after,matches,num_matches*sizeof(matches[0]));
  return -((num_matches<<16)|(after-ind));
  }

void search_re(windowstuff *redia,windowstuff *map,
               windowstuff *xover,windowstuff *pane[2])
  {
  redia_desc *rd = redia->local->desc.redia;

  int rec_ec = 0;
  PCRE2_SIZE rec_eo;
  pcre2_code *rec = pcre2_compile(rd->re, rd->len_re,
                                  0, &rec_ec, &rec_eo,
                                  NULL);
  pcre2_match_data *md = pcre2_match_data_create_from_pattern(rec,NULL);
  PCRE2_SIZE *ov = pcre2_get_ovector_pointer(md);
  
  pane_desc *pd[2] = { pane[0]->local->desc.pane,
                       pane[1]->local->desc.pane };

  memset(rd->matches,0,rd->size_matches*sizeof(rd->matches[0]));
  for (int side=0;side<2;side++)
    {
    fileinfo *file = & pd[side]->dif->file[side];
    memset(file->matches,0,file->lines*sizeof(file->matches[0]));
    int lines = file->lines;
    for (int line=0;line<lines;line++)
      {
      int start=0,options=0,num_matches=0;
      int32_t matches[16];
      char *sol = file->line[line];
      char *eol = file->line[line+1]-1;
      while ( num_matches<16 &&
              pcre2_match(rec,sol,eol-sol,start,options,md,NULL) >= 0)
        {
        if (ov[1]>0xefff) break;
        matches[num_matches++] = (ov[0]<<16)|ov[1];
        if (0) printf("side %d line %d pos %d-%d\n",side,line,ov[0],ov[1]);
        start = ov[1];
        }
      file->matches[line] = find_or_make_match(num_matches,matches,rd);
      }
    }
  
  pcre2_match_data_free(md);
  pcre2_code_free(rec);

  draw_map(1,map->local->desc.map);
  draw_xover(1,xover->local->desc.xover);

  for (int s=0;s<2;s++)
    {
    pane[s]->local->desc.pane->matches = rd->matches;
    draw_pane(1,pane[s]->local->desc.pane);
    }
  }


void draw_redia(int expose,redia_desc *rd)
  {
  xclear(rd->w);
  if (rd->re && rd->len_re)
    xstring(rd->w,BORDER,rd->fontinfo->ascent,
            rd->re,rd->len_re);
  }

void destroy_redia(windowstuff **rv)
  {
  disassoc_win((*rv));
  free((*rv)->local->desc.map);
  free((*rv)->local);
  closewindow(*rv);
  *rv = NULL;
  }


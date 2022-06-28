#include "Xstuff.h"
#include "proto.h"

void draw_pane(int expose,windowstuff *win)
  {
  pane_desc *pd = win->local->desc.pane;
  int height = (pd->fontinfo->ascent+pd->fontinfo->descent);
  int fl = pd->w->hint.height/height;
  int width = (pd->fontinfo->min_bounds.width);
  int fw = pd->w->hint.width/(width ? width : 1);
  char **line = pd->dif->file[pd->side].line;
  int32_t *matches = pd->dif->file[pd->side].matches;
  int l=pd->dif->file[pd->side].lines;
  if (expose) pd->pstartline= -1;

  int rm = pd->startline-pd->pstartline;
  
  if (rm == 0) return;

  int start,end;
  if (fl-rm>0 && rm>0 && pd->pstartline>=0)
    {
    XCopyArea(pd->w->display,pd->w->window,pd->w->window,pd->w->gc,
              0,rm*height,
              pd->w->hint.width,(fl-rm)*height,
              0,0);
    XClearArea(pd->w->display,pd->w->window,
               0,(fl-rm)*height,
               pd->w->hint.width,rm*height,
               False);
    start=fl-rm;
    end=fl;
    }
  else if (fl+rm>0 && rm<0 && pd->pstartline>=0)
    {
    XCopyArea(pd->w->display,pd->w->window,pd->w->window,pd->w->gc,
              0,0,
              pd->w->hint.width,(fl+rm)*height,
              0,-rm*height);
    XClearArea(pd->w->display,pd->w->window,
               0,0,
               pd->w->hint.width,-rm*height,
               False);
    start=0;
    end= -rm;
    }
  else
    {
    xclear(pd->w);
    start=0;
    end=fl;
    }
  
  pd->pstartline=pd->startline;
  for (int i=start;i<end;i++)
    {
    int inpane = i+pd->startline<l && i+pd->startline>=0 ;
    char *theline = inpane ? line[i+pd->startline] : "~";
    char *eol = inpane ? line[i+pd->startline+1]-1 : theline+1;

    int b=0;
    int ia=0;
    for (int j=0;j<pd->dif->diffs;j++)
      {
      b |= pd->dif->diff[j].fs[pd->side]<=i+pd->startline
           && pd->dif->diff[j].fe[pd->side]>i+pd->startline ;
      ia |= pd->dif->diff[j].fs[pd->side]<=i+pd->startline
            && pd->dif->diff[j].fe[pd->side]==i+pd->startline ;
      }

    int ll = eol-theline;
    if (eol>theline)
      {
      char buffer[fw+1];
      int sl;
      char *s = rendertabs(theline,eol-theline,buffer,fw,&sl);
      xistring(pd->w,INDENT,i*height+pd->fontinfo->ascent, s ,sl);
      
      int32_t match = inpane ? matches[i+pd->startline] : 0;
      int nm = get_num_matches(match);
      if (nm)
        {
        XSetFunction(pd->w->display,pd->w->gc,FunnyMode);
        XSetForeground(pd->w->display,pd->w->gc,
                       pd->w->foreground|pd->w->background);
        
        for (int m=0;m<nm;m++)
          {
          int32_t msted = get_match(match,pd->matches,m);
          int beg = msted>>16;
          int end = msted&0xffff;

          int direction_return,font_ascent_return,font_descent_return;
          XCharStruct pre_return;
          XTextExtents(pd->fontinfo, s, beg,
                       &direction_return,&font_ascent_return,
                       &font_descent_return, &pre_return);
          int xstart = pre_return.width;
          
          XCharStruct post_return;
          XTextExtents(pd->fontinfo, s, end,
                       &direction_return,&font_ascent_return,
                       &font_descent_return, &post_return);
          int xwidth = post_return.width - xstart;

          XFillRectangle(pd->w->display,pd->w->window,pd->w->gc,
                         INDENT-1+xstart,i*height,
                         xwidth+1,height);
          }
        
        pencolor(pd->w,1);
        XSetFunction(pd->w->display,pd->w->gc,GXcopy);
        }
      }
    
    if (b)
      {
      XSetFunction(pd->w->display,pd->w->gc,FunnyMode);
      XSetForeground(pd->w->display,pd->w->gc,pd->w->foreground|pd->w->background);
      XFillRectangle(pd->w->display,pd->w->window,pd->w->gc,0,i*height,
                     pd->w->hint.width,height);
      pencolor(pd->w,1);
      XSetFunction(pd->w->display,pd->w->gc,GXcopy);
      }

    if (ia)
      {
      XSetFunction(pd->w->display,pd->w->gc,FunnyMode);
      XSetForeground(pd->w->display,pd->w->gc,pd->w->foreground|pd->w->background);
      XDrawLine(pd->w->display,pd->w->window,pd->w->gc,0,i*height,
                     pd->w->hint.width,i*height);
      pencolor(pd->w,1);
      XSetFunction(pd->w->display,pd->w->gc,GXcopy);
      }
    }
  }

windowstuff *create_pane(windowstuff *main,int x,int y,int w,int l,
                         int id,diffs *dfs,XFontStruct *fontinfo)
  {
  windowstuff *rv;
  
  rv = openwindow(main,"Text Pane",PPosition,x,y,w,l,-1,0,
                  ExposureMask|StructureNotifyMask,
                  draw_pane);
  assoc_win(rv);
  rv->local = newitem(local_type,1);
  rv->local->object = PANE;
  rv->local->desc.pane = newitem(pane_desc,1);
  rv->local->desc.pane->w = rv;
  rv->local->desc.pane->fontinfo = fontinfo;
  rv->local->desc.pane->side = id;
  rv->local->desc.pane->dif = dfs;
  rv->local->desc.pane->startline=0;
  rv->local->desc.pane->pstartline= -1;
  XSetFont(rv->display,rv->gc,fontinfo->fid);
  draw_pane(0,rv);
  return(rv);
  }

void destroy_pane(windowstuff **rv)
  {
  disassoc_win((*rv));
  free((*rv)->local->desc.pane);
  free((*rv)->local);
  closewindow(*rv);
  *rv = NULL;
  }




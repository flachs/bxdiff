#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "rbtree.h"
#include "space_shift.h"

// stdint.h
// keys:   uint32_t uint64_t int32_t int64_t double charary    void *
// cmp :   ARI      ARI      ARI     ARI     ARI    cust(&KEY) cust(key)


static rb_node_t *rb_free_list[64];

rb_node_t *rb_new_node(size_t size)
  {
  int s = SPACE_SHIFT(sizeof(rb_node_t)+size);
  rb_node_t *rv;
  
  if (rb_free_list[s])
    {
    rv = rb_free_list[s];
    rb_free_list[s] = rv->parent;
    }
  else
    {
    rv = malloc(1<<s);
    rv->m[0] = 0xff;
    rv->m[1] = 'r';
    rv->m[2] = 0 ;
    rv->s = s;
    }

  rv->red = 1;
  memset(& rv->parent, 0, (1<<s) - offsetof(rb_node_t,parent));
  
  return rv;
  }

void rb_recycle_node(rb_node_t *n)
  {
  n->parent = rb_free_list[n->s];
  rb_free_list[n->s] = n;
  }
    
void rb_print_tree(rb_tree_t *t,FILE *stream,int level,char label,rb_node_t *n)
  {
  if (!n) return ;
  
  for (int i=0;i<level;i++) fputs("  ",stream);
  fprintf(stream,"%c %c key=",label,n->red ? 'r' : 'b');
  t->fprint_key(stream,n);
  fputs(" value=",stream);
  t->fprint_value(stream,n);
  fputc('\n',stream);
  rb_print_tree(t,stream,level+1,'l',n->tree[0]);
  rb_print_tree(t,stream,level+1,'g',n->tree[1]);
  }

#if 0
#define VALUE(n) (*(VALUE_TYPE *)(n->data))
#define SET_VALUE(n,v) VALUE(n) = v
#define FPRINT_KEY(s,k) fprintf(s,"%d",k)
#define FPRINT_VALUE(s,v) fprintf(s,"%d",v)
#endif

#define RB_GEN 1

#include "rbtree.h"

#define PREFIX rb
#define CUST_KEY_CMP 0
#define KEY(n) (*(KEY_TYPE *)(n->data))

#define MAKEEM(fmt)                                \
void NAME(fprint_key)(FILE *stream,rb_node_t *n) \
  { \
  fprintf(stream,fmt,KEY(n)); \
  } \
\
rb_tree_t *NAME(new_tree)(rb_fprint_ft print_value)\
  {\
  rb_tree_t *rv = malloc(sizeof(rb_tree_t));\
  memset(rv,0,sizeof(rv));\
  rv->find = NAME(find);\
  rv->add  = NAME(add);\
  rv->del  = NAME(del);\
  rv->fprint_key   = NAME(fprint_key);\
  rv->fprint_value = print_value;\
  return rv ;\
  }\


#define KEY_TYPE   int32_t
#include "rbtree.h"
MAKEEM("%d");
#undef  KEY_TYPE

#define KEY_TYPE   uint32_t
#include "rbtree.h"
MAKEEM("%u");
#undef  KEY_TYPE

#define KEY_TYPE   int64_t
#include "rbtree.h"
MAKEEM("%lld");
#undef  KEY_TYPE

#define KEY_TYPE   uint64_t
#include "rbtree.h"
MAKEEM("%08llx");
#undef  KEY_TYPE

#define KEY_TYPE   double
#include "rbtree.h"
MAKEEM("%g");
#undef  KEY_TYPE

#undef  CUST_KEY_CMP
#define CUST_KEY_CMP 1
#define KEY_TYPE voidp
#include "rbtree.h"
rb_tree_t *NAME(new_tree)(rb_cmp_ft cmpf,
                          rb_fprint_ft print_key,
                          rb_fprint_ft print_value)
  {
  rb_tree_t *rv = malloc(sizeof(rb_tree_t));
  memset(rv,0,sizeof(rv));
  rv->find = NAME(find);
  rv->add  = NAME(add);
  rv->del  = NAME(del);
  rv->cmpf = cmpf;
  rv->fprint_key   = print_key;
  rv->fprint_value = print_value;
  return rv ;
  }

#undef  KEY_TYPE
#define KEY_TYPE charblock
#undef  KEY
#define KEY(n) n->data
#include "rbtree.h"
rb_tree_t *NAME(new_tree)(rb_cmp_ft cmpf,
                          rb_fprint_ft print_key,
                          rb_fprint_ft print_value)
  {
  rb_tree_t *rv = malloc(sizeof(rb_tree_t));
  memset(rv,0,sizeof(rv));
  rv->find = NAME(find);
  rv->add  = NAME(add);
  rv->del  = NAME(del);
  rv->cmpf = cmpf;
  rv->fprint_key   = print_key;
  rv->fprint_value = print_value;
  return rv ;
  }
#undef  KEY
#undef  KEY_TYPE
#undef  CUST_KEY_CMP


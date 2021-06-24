
#ifndef __rbtree_h_
#define __rbtree_h_


typedef struct rb_node_s
  {
  char m[5],k,s,red;
  struct rb_node_s *parent,*tree[2],*list[2];
  unsigned char data[0] __attribute__((aligned(16)));
  } rb_node_t;

typedef void *voidp;
typedef void *charblock;
typedef int (*rb_cmp_ft)();
typedef rb_node_t *(*rb_op_ft)();
typedef void (*rb_fprint_ft)(FILE *stream,rb_node_t *n);

#define RB_CMP_FN_ARI ((rb_cmp_ft)(0))

// cmpf (key type) 0->unsigned 1->signed 2->double else ptr to cmp
typedef struct rb_tree_s
  {
  char m[4];
  int num;
  rb_node_t *root;
  rb_node_t first,last;
  rb_cmp_ft cmpf;
  rb_op_ft find,add,del;
  rb_fprint_ft fprint_key,fprint_value;
  } rb_tree_t;

#define rb_doit(t,f,n) (*(t->f))(t,n)
#define rb_find(t,n) rb_doit(t,find,n)
#define rb_add(t,n)  rb_doit(t,add,n)
#define rb_del(t,n)  rb_doit(t,del,n)

typedef __uint128_t uint128_t;

rb_node_t *rb_new_node(size_t size);
void rb_recycle_node(rb_node_t *n);
void rb_print_tree(rb_tree_t *t,FILE *stream,int level,char label,rb_node_t *n);

rb_tree_t *rb_new_tree_int32_t(rb_fprint_ft print_value);
rb_tree_t *rb_new_tree_uint32_t(rb_fprint_ft print_value);
rb_tree_t *rb_new_tree_int64_t(rb_fprint_ft print_value);
rb_tree_t *rb_new_tree_uint64_t(rb_fprint_ft print_value);
rb_tree_t *rb_new_tree_double(rb_fprint_ft print_value);
rb_tree_t *rb_new_tree_voidp(rb_cmp_ft cmpf,
                             rb_fprint_ft print_key,
                             rb_fprint_ft print_value);
rb_tree_t *rb_new_tree_charblock(rb_cmp_ft cmpf,
                                 rb_fprint_ft print_key,
                                 rb_fprint_ft print_value);


#endif

#ifdef RB_GEN

#ifndef __RB_TREE_GEN_
#define __RB_TREE_GEN_

#define TREE_LT 0
#define TREE_GT 1
#define LIST_P  0
#define LIST_N  1

#define TREE(d) tree[d]
#define LIST(d) list[d]

#define UP(n) (n)->parent
#define LT(n) (n)->tree[TREE_LT]
#define GT(n) (n)->tree[TREE_GT]
#define PL(n) (n)->list[LIST_P]
#define NL(n) (n)->list[LIST_N]

#define GL_UE3(x,y,z) x ## _ ## y ## _ ## z
#define GLUE3(x,y,z) GL_UE3(x,y,z)
#define NAME(n) GLUE3(PREFIX,n,KEY_TYPE)

static inline int is_red(rb_node_t *n)
  {
  return n && n->red;
  }

static inline rb_node_t *rb_rotate(rb_node_t *n, int dir)
  {
  rb_node_t *t = n->tree[!dir];

  n->tree[!dir] = t->tree[dir];
  t->tree[dir] = n;
  
  UP(t) = UP(n);
  UP(n) = t;
  if (n->tree[!dir]) UP(n->tree[!dir]) = n;
  
  n->red = 1;
  t->red = 0;

  return t;
  }

static inline rb_node_t *rb_double(rb_node_t *n, int dir)
  {
  n->tree[!dir] = rb_rotate(n->tree[!dir],!dir);
  return rb_rotate(n,dir);
  }

static inline rb_node_t *rb_greatest(rb_node_t *n)
  {
  while (n->tree[1])
    n = GT(n);
  return n;
  }


static void rb_del_balance(rb_node_t *n, int dir)
  {
  int done=0;

  while (!done)
    {
    rb_node_t *p = n;
    rb_node_t *g= UP(n);
    rb_node_t *rvp = 0;
    int rvdir = 0;

    if (!g) return; // empty tree
    
    if (g->m[0] == -1)
      {
      rvdir = GT(g) == n;
      rvp = g;
      }
    else
      {
      rvdir = 0;
      rvp = g;
      }
    
    rb_node_t *s = p->tree[!dir];
    
    /* Case reduction, remove red sibling */
    if (is_red(s))
      {
      n = rb_rotate(n, dir);
      s = p->tree[!dir];
      }
    
    if (s != NULL)
      {
      /* Black sibling cases */
      if (!is_red(LT(s)) && !is_red(GT(s)))
        {
        if (is_red(p))
          {
          done = 1;
          }

        p->red = 0;
        s->red = 1;
        }
      else
        {
        int save = n->red;
        int nn = (n == p);

        if (is_red(s->tree[!dir]))
          {
          p = rb_rotate(p, dir);
          }
        else
          {
          p = rb_double(p, dir);
          }

        p->red = save;
        LT(p)->red = 0;
        GT(p)->red = 0;

        if (nn)
          {
          n = p;
          }
        else
          {
          n->tree[dir] = p;
          }
        
        done = 1;
        }
      }

    if (rvp)
      {
      rvp->tree[rvdir] = n;
      }
    
    g = UP(n);
    if (g->m[0] != -1) return;
    dir = GT(g) == n;
    n = g;
    }
  }
#else

#if (CUST_KEY_CMP)
#define CMP(k1,k2) ((*tree->cmpf)(k1,k2))
#else
#define CMP(k1,k2) ((k1>k2) ? 1 : (k1<k2) ? -1 : 0)
#endif

static rb_node_t *NAME(find)(rb_tree_t *tree,KEY_TYPE key)
  {
  rb_node_t *rv = tree->root;
  
  while (rv)
    {
    int dir = CMP(key,KEY(rv));
    if (!dir) return(rv);
    dir = (dir + 1)>>1; // -1 or 1 -> 0 or 1
    rv = rv->TREE(dir);
    }
  return rv;
  }


static rb_node_t *NAME(add)(rb_tree_t *tree, rb_node_t *m)
  {
  rb_node_t *rv  = 0;      // replaced node if any
  
  if (tree->root == NULL)
    { /* Empty tree case */
    tree->root = m;
    NL(m) = & tree->last;
    PL( &tree->last ) = m;
    PL(m) = & tree->first;
    NL( &tree->first ) = m;
    m->red = 0;
    return rv;
    }

  rb_node_t head = { 0 }; /* False tree root */

  rb_node_t *g;     /* Grandparent & parent */
  rb_node_t *p, *q;     /* Iterator & parent */
  int dir = 0, last;

  /* Set up helpers */
  rb_node_t *t = &head;
  g = p = NULL;
  q = t->tree[1] = tree->root;

  /* Search down the tree */
  while (1)
    {
    if (!q)
      { /* Insert new node at the bottom */
      p->tree[dir] = q = m;
      UP(m) = p;
      
      // hookup link list
      // if dir>0 p<m => p       -> m -> p->next
      // if dir<0 m<p => p->prev -> m -> p
      m->LIST(!dir) = p;
      m->LIST( dir) = p->LIST(dir);
      p->LIST( dir) = m;
      }
    else if (is_red(q->tree[0]) && is_red(q->tree[1]))
      {
      /* Color flip */
      q->red = 1;
      LT(q)->red = 0;
      GT(q)->red = 0;
      }

    /* Fix red violation */
    if (is_red(q) && is_red(p))
      {
      int dir2 = t->tree[1] == g;

      if (q == p->tree[last])
        {
        t->tree[dir2] = rb_rotate(g, !last);
        }
      else
        {
        t->tree[dir2] = rb_double(g, !last);
        }
      }

    if (q==m) break;
    
    last = dir;

    dir = CMP(KEY(m),KEY(q));
    
    if (!dir)
      { /* found the key -- replace q with m */
      m->red = q->red;

      // fix tree links
      UP(m) = UP(q);
      if (UP(m)) p->tree[last] = m;
      else head.tree[1] = m;
      
      LT(m) = LT(q);
      GT(m) = GT(q);
      if (LT(m)) UP(LT(m)) = m;
      if (GT(m)) UP(GT(m)) = m;

      // fix list links
      PL(m) = PL(q);
      NL(m) = NL(q);
      NL(PL(m)) = m;
      PL(NL(m)) = m;
      
      rv = q;
      break;
      }
    
    dir = (dir + 1)>>1; // -1 or 1 -> 0 or 1
    
    /* Update helpers */
    if (g != NULL)
      {
      t = g;
      }

    g = p, p = q;
    q = q->tree[dir];
    }

  /* Update root */
  tree->root = head.tree[1];

  /* Make root black */
  tree->root->red = 0;

  return rv;
  }

static void NAME(del_node)(rb_node_t *n,rb_node_t *top)
  {
  rb_node_t *p = UP(n);
  int dir = GT(p)==n;

  // fix linked list
  NL(PL(n)) = NL(n);
  PL(NL(n)) = PL(n);
  
  int ltn = LT(n) == NULL;
  int gtn = GT(n) == NULL;

  if (ltn && gtn)
    { // n is leaf just remove it
    p->tree[dir] = 0;
    if (!is_red(n)) rb_del_balance(p, dir);
    return;
    }
  
  if (ltn || gtn)
    { // only 1 side of n is populated
    rb_node_t *save = n->tree[ltn];
    p->tree[dir] = save;
    UP(save) = p;

    if (is_red(n))
      { // can just delete red node
      return;
      }
    
    if (is_red(save))
      {
      save->red = 0;
      return;
      }
    
    rb_del_balance(p, dir);
    return;
    }

  // both sides of n are populated
  rb_node_t *h = rb_greatest(n->tree[0]);        // find greatest node less than
  rb_node_t *hp = UP(h);
  rb_node_t *hc = LT(h);
  int hpdir = 1;
  
  p->tree[dir] = h;
  UP(h) = p;

  GT(h) = GT(n);
  UP(GT(n)) = h;
    
  if (hp==n)
    {
    hp = h;
    hpdir = 0;
    }
  else
    {
    LT(h) = LT(n);
    UP(LT(n)) = h;
    
    if (hc)
      {
      UP(hc) = hp;
      }
    GT(hp) = hc;
    }  

  if (hc)
    {
    if (is_red(h))
      {
      h->red = n->red;
      return;
      }
    
    if (is_red(hc))
      {
      hc->red = 0;
      return;
      }
    
    rb_del_balance(h, dir);
    return;
    }
  else
    { // h is leaf
    if (is_red(h))
      {
      h->red = n->red;
      return;
      }
    h->red = n->red;
    rb_del_balance(hp, hpdir);
    }
  }

static rb_node_t *NAME(del)(rb_tree_t *tree, rb_node_t *n)
  {
  int done = 0;

  rb_node_t fake,*fp;
  fp = &fake;
  memset(fp,0,sizeof(fake));
  
  fp->tree[0] = tree->root;
  UP(tree->root) = fp;
  
  NAME(del_node)(n,fp);

  tree->root = fp->tree[0];
  if (tree->root)
    {
    tree->root->red = 0;
    UP(tree->root) = 0;
    }

  return n;
  }

#undef CMP
#endif
#endif

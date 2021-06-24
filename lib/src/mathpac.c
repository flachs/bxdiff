#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include "mathpac.h"

/********
nrerror.c
********/

/*EP*/ void nrerror(char *error_text)
  {
  void exit();

  fprintf(stderr,"mathpac run-time error...\n");
  fprintf(stderr,"%s\n",error_text);
  exit(1);
  }


/*******
cos2(theta) -- bkf
*******/

/*EP*/ double cos2(double theta)
  {
  double c;
  c=cos(theta);
  return(c*c);
  }

/*******
d2matrix -- bkf
*******/

/*EP*/ void allocd2matrixdata(d2matrix *rv,int zero)
  {
  int i;
  
  if (rv)
    {
    rv->data = newitem(double *,rv->nr);
    if (zero) for (i=0;i<rv->nr;i++) rv->data[i] = newitem(double,rv->nc);
    else for (i=0;i<rv->nr;i++) rv->data[i] = newitem_ni(double,rv->nc);
    }
  }

/*EP*/ d2matrix *new_d2matrix(int nr,int nc,int zero)
  {
  int i;
  d2matrix *rv;
  
  rv = newitem(d2matrix,1);
  rv->dimen=2;
  rv->nr = nr;
  rv->nc = nc;
  allocd2matrixdata(rv,zero);
  return(rv);
  }

/*EP*/ void free_d2matrixdata(d2matrix *rv)
  {
  int i;
  
  for (i=0;i<rv->nr;i++) free(rv->data[i]);
  free(rv->data);
  rv->data=NULL;
  }

/*EP*/ void free_d2matrix(d2matrix *rv)
  {
  free_d2matrixdata(rv);
  free(rv);
  }

/*EP*/ void fprint_d2matrix(FILE *f,d2matrix *m)
  {
  int i,j;
  
  for (i=0;i<m->nr;i++)
    {
    for (j=0;j<m->nc;j++)
      fprintf(f,"%g ",m->data[i][j]);
    fprintf(f,"\n");
    }
  }

/*EP*/ d2matrix *fscan_d2matrix(FILE *f)
  {
  int i,j,n,m;
  d2matrix *rv;

  fscanf(f,"%d%d",&n,&m);
  rv=new_d2matrix(n,m,1);
  
  for (i=0;i<rv->nr;i++)
    {
    for (j=0;j<rv->nc;j++)
      fscanf(f,"%lg",&(rv->data[i][j]));
    }
  return(rv);
  }

/*EP*/ d2matrix *d2matrix_copy(d2matrix *m)
  {
  int i;
  d2matrix *rv;
  
  rv = new_d2matrix(m->nr,m->nc,0);
  
  for (i=0;i<rv->nr;i++) 
    memmove(rv->data[i],m->data[i],rv->nc*sizeof(double));

  return(rv);
  }

/*EP*/ d2matrix *constantd2matrix(int nr,int nc, ...)
  {
  va_list ap;
  int i,j;
  d2matrix *rv;
  double *d;
  
  va_start(ap,nc);
  rv = new_d2matrix(abs(nr),abs(nc),0);

  if (nr<0)
    {
    for (i=0;i<rv->nr;i++) 
      for (j=0;j<rv->nc;j++) rv->data[i][j]= va_arg(ap,double);
    }
  else
    {
    d = va_arg(ap,double *);
    for (i=0;i<rv->nr;i++)
      for (j=0;j<rv->nc;j++) rv->data[i][j] = *d++;
    }
  va_end(ap);
  return(rv);
  }

/*EP*/ d2matrix *identityd2matrix(int n)
  {
  d2matrix *rv;
  int i;
  
  rv = new_d2matrix(n,n,1);
  for (i=0;i<n;i++) rv->data[i][i]=1;
  return(rv);
  }


/*EP*/ d2matrix *diagonald2matrix(int n,...)
  {
  va_list ap;
  int i;
  d2matrix *rv;
  double *d;
  
  va_start(ap,n);
  rv = new_d2matrix(abs(n),abs(n),1);
  if (n<0)
    {
    n= -n;
    for (i=0;i<n;i++) rv->data[i][i] = va_arg(ap,double);
    }
  else
    {
    d = va_arg(ap,double *);
    for (i=0;i<n;i++) rv->data[i][i] = d[i];
    }
  va_end(ap);
  return(rv);
  }

/*EP*/ d2matrix *d2matrixdestprep(d2matrix *provided,int nr,int nc)
  {
  int i;
  d2matrix *rv;
  
  if (provided==NULL) 
    {
    rv = new_d2matrix(nr,nc,1);
    rv->temp=1;
    return(rv);
    }
  if (provided->nr!=nr || provided->nc!=nc)
    {
    if (provided->data) free_d2matrixdata(provided);
    provided->nr=nr;
    provided->nc=nc;
    allocd2matrixdata(provided,1);
    return(provided);
    }
  for (i=0;i<nr;i++) memset(provided->data[i],0,sizeof(double)*nc);
  return(provided);
  }


/*EP*/ d2matrix *d2matrix_sum(int n,...)
  {
  d2matrix *rv,**list;
  int i,j,k,nr,nc,destprep;
  double s;
  va_list ap;
  
  va_start(ap,n);
  if (n<1) nrerror("d2matrix_sum no addends");

  list = newitem(d2matrix *,2);
  rv = va_arg(ap,d2matrix *);
  
  list[0] = va_arg(ap,d2matrix *);
  nr = list[0]->nr;
  nc = list[0]->nc;

  destprep = (list[0]==rv) ? 0 :1;
  for (k=1;k<n;k++)
    {
    list[k] = va_arg(ap,d2matrix *);
    if (list[k]==rv) destprep=0;
    if (nr!=list[k]->nr || nc!=list[k]->nc) 
      nrerror("d2matrix_sum dimensions don't match");
    }
  va_end(ap);

  if (destprep) rv = d2matrixdestprep(rv,nr,nc);
  for (i=0;i<nr;i++) for (j=0;j<nc;j++)
    {
    for (k=0,s=0;k<n;k++) s += list[k]->data[i][j];
    rv->data[i][j] = s;
    }
  
  for (k=0;k<n;k++) if (list[k]->temp) free_d2matrix(list[k]);
  free(list);
  return(rv);
  }


/*EP*/ d2matrix *d2matrix_mult(d2matrix *d,d2matrix *a,d2matrix *b)
  {
  d2matrix *rv;
  int i,j,k,n,nr,nc,nl;
  double s;
  
  if (a->nc != b->nr) nrerror("d2matrix_mult dimensions don't match");
  
  nr = a->nr;
  nc = b->nc;
  nl = a->nc;
    
  rv = d2matrixdestprep(d,nr,nc);

  for (i=0;i<nr;i++)
    for (j=0;j<nc;j++)
      for (k=0;k<nl;k++)
        rv->data[i][j] += a->data[i][k] * b->data[k][j];
  
  if (a->temp) free_d2matrix(a);
  if (b->temp) free_d2matrix(b);
  return(rv);
  }


/*EP*/ d2matrix *d2matrix_scale(d2matrix *d,d2matrix *a,double c)
  {
  d2matrix *rv;
  int i,j,k,n,nr,nc;
  double s;
  
  nr = a->nr;
  nc = a->nc;
    
  rv = d2matrixdestprep(d,nr,nc);

  for (i=0;i<nr;i++)
    for (j=0;j<nc;j++)
      rv->data[i][j] = a->data[i][j]*c;
  
  if (a->temp) free_d2matrix(a);
  return(rv);
  }

/*EP*/ d2matrix *d2matrix_trans(d2matrix *d,d2matrix *a)
  {
  d2matrix *rv;
  int i,j,k,n,nr,nc;
  double s;
  
  nr = a->nc;
  nc = a->nr;
    
  rv = d2matrixdestprep(d,nr,nc);

  for (j=0;j<nc;j++)
    for (i=0;i<nr;i++)
      rv->data[i][j] = a->data[j][i];
  
  if (a->temp) free_d2matrix(a);
  return(rv);
  }

/*EP*/ d2matrix *d2matrix_inverse(d2matrix *d,d2matrix *a)
  {
  d2matrix *rv,*tmp;
  int *indx;
  int i,j,k,n,nr,nc,dt;
  double s,*col;
  

  if (a->nc != a->nr) nrerror("mathpac:d2matrixinverse matrix not square");
  n = a->nr;

  indx = newitem_ni(int,n);
  col = newitem_ni(double,n);
  
  rv = d2matrixdestprep(d,n,n);
  tmp = d2matrix_copy(a);
  ludcmp(tmp,indx,&dt);
  
  
  for (j=0;j<n;j++)
    {
    for (i=0;i<n;i++)  col[i]=0;
    col[j]=1;
    lubksb(tmp,indx,col);
    for (i=0;i<n;i++)  rv->data[i][j] = col[i];
    }
  
  if (a->temp) free_d2matrix(a);
  free_d2matrix(tmp);
  free(col);
  free(indx);
  return(rv);
  }

/*EP*/ int d2matrix_cmp(d2matrix *a,d2matrix *b)
  {
  int i;
  
  if (a==b) return(-1);
  if (a==NULL || b==NULL) return(0);
  if (a->nr != b->nr || a->nc != b->nc) return(0);
  for (i=0;i<a->nr;i++) 
    if (bcmp(a->data[i],b->data[i],sizeof(double)*a->nr)) return(0);
  return(1);
  }


/*EP*/ d2matrix *d2matrix_sub(d2matrix *d,d2matrix *a,d2matrix *b)
  {
  d2matrix *rv;
  int i,j,k,n,nr,nc;
  double s;
  
  if (a->nc != b->nc || a->nr != b->nr) 
    nrerror("d2matrix_sub dimensions don't match");
  
  nr = a->nr;
  nc = b->nc;
    
  rv = d2matrixdestprep(d,nr,nc);

  for (i=0;i<nr;i++)
    for (j=0;j<nc;j++)
      rv->data[i][j] = a->data[i][j] - b->data[i][j];
  
  if (a->temp) free_d2matrix(a);
  if (b->temp) free_d2matrix(b);
  return(rv);
  }


/********
bico.c
********/

/*EP*/ double bico(long n,long k)
  {
  return( floor(0.5+exp(factln(n)-factln(k)-factln(n-k))));
  }


/********
factln.c
********/

/*EP*/ double factln(long n)
  {
  static double a[101];

  if (n < 0) nrerror("Negative factorial in routine FACTLN");
  if (n <= 1) return(0.0);
  if (n <= 100) return(a[n] ? a[n] : (a[n]=gammln(n+1.0)));
  else return( gammln(n+1.0));
  }


/********
gammln.c
********/

/*EP*/ double gammln(double xx)
  {
  double x,tmp,ser;
  static double cof[6]={76.18009173,-86.50532033,24.01409822,
    -1.231739516,0.120858003e-2,-0.536382e-5};
  int j;

  x=xx-1.0;
  tmp=x+5.5;
  tmp -= (x+0.5)*log(tmp);
  ser=1.0;
  for (j=0;j<=5;j++) 
    {
    x += 1.0;
    ser += cof[j]/x;
    }
  return( -tmp+log(2.50662827465*ser));
  }




/********
bicoln.c
********/

/*EP*/ double bicoln(long n,long k)
  {
  return( factln(n)-factln(k)-factln(n-k));
  }


/********
ran3.c
********/


/*EP*/ double ran3(int *idum)
  {
  static ran3state state;

  return(ran3ns(idum,&state));
  }



#define MBIG 1000000000
#define MSEED 161803398
#define MZ 0
#define FAC (1.0/MBIG)

/*EP*/ double ran3ns(int *idum,ran3state *state)
  {
  long mj,mk;
  int i,ii,k;

  if (*idum < 0 || state->iff == 0) 
    {
    state->iff=1;
    mj=MSEED-(*idum < 0 ? -*idum : *idum);
    mj %= MBIG;
    state->ma[55]=mj;
    mk=1;
    
    for (i=1;i<=54;i++) 
      {
      ii=(21*i) % 55;
      state->ma[ii]=mk;
      mk=mj-mk;
      if (mk < MZ) mk += MBIG;
      mj=state->ma[ii];
      }
    
    for (k=1;k<=4;k++)
      for (i=1;i<=55;i++) 
        {
        state->ma[i] -= state->ma[1+(i+30) % 55];
        if (state->ma[i] < MZ) state->ma[i] += MBIG;
        }
      
    state->inext=0;
    state->inextp=31;
    *idum=1;
    }
  
  if (++state->inext == 56) state->inext=1;
  if (++state->inextp == 56) state->inextp=1;
  mj=state->ma[state->inext]-state->ma[state->inextp];
  if (mj < MZ) mj += MBIG;
  state->ma[state->inext]=mj;
  return mj*FAC;
  }

#undef MBIG
#undef MSEED
#undef MZ
#undef FAC


/********
ran1.c
********/
#define M1 259200
#define IA1 7141
#define IC1 54773
#define RM1 (1.0/M1)
#define M2 134456
#define IA2 8121
#define IC2 28411
#define RM2 (1.0/M2)
#define M3 243000
#define IA3 4561
#define IC3 51349

/*EP*/ double ran1(int *idum)
  {
  static long ix1,ix2,ix3;
  static double r[98];
  double temp;
  static int iff=0;
  int j;

  if (*idum < 0 || iff == 0) 
    {
    iff=1;
    ix1=(IC1-(long)(*idum)) % M1;
    ix1=(IA1*ix1+IC1) % M1;
    ix2=ix1 % M2;
    ix1=(IA1*ix1+IC1) % M1;
    ix3=ix1 % M3;
    for (j=1;j<=97;j++) 
      {
      ix1=(IA1*ix1+IC1) % M1;
      ix2=(IA2*ix2+IC2) % M2;
      r[j]=(ix1+ix2*RM2)*RM1;
      }
    *idum=1;
    }
  ix1=(IA1*ix1+IC1) % M1;
  ix2=(IA2*ix2+IC2) % M2;
  ix3=(IA3*ix3+IC3) % M3;
  j=1 + ((97*ix3)/M3);
  if (j > 97 || j < 1) nrerror("RAN1: This cannot happen.");
  temp=r[j];
  r[j]=(ix1+ix2*RM2)*RM1;
  return temp;
  }

#undef M1
#undef IA1
#undef IC1
#undef RM1
#undef M2
#undef IA2
#undef IC2
#undef RM2
#undef M3
#undef IA3
#undef IC3


/********
gasdev.c
********/

/*EP*/ double gasdev(int *idum)
  {
  static gasdevstate state;

  return(gasdevns(idum,&state));
  }


/*EP*/ double gasdevns(int *idum,gasdevstate *state)
  {
  double fac,r,v1,v2;

  if (state->iset == 0) 
    {
    do 
      {
      v1=2.0*ran3ns(idum,&state->state)-1.0;   /* used to be */
      v2=2.0*ran3ns(idum,&state->state)-1.0;   /* ran1(idum) */
      r=v1*v1+v2*v2;
      } 
    while (r >= 1.0);
    fac=sqrt(-2.0*log(r)/r);
    state->gset=v1*fac;
    state->iset=1;
    return v2*fac;
    } 
  else 
    {
    state->iset=0;
    return state->gset;
    }
  }




/******
mrqcof.c
*******/

/*EP*/ void mrqcof
  (
  double *x,
  double *y,
  double *sig,
  int ndata,
  double *a,
  int ma,
  int *lista,
  int mfit,
  d2matrix *alpha,
  double *beta,
  double *chisq,
  double (*funcs)()
  )
  {
  int k,j,i;
  double ymod,wt,sig2i,dy;
  static double *dyda;
  static int dim_dyda;

  if (dyda && dim_dyda!=ma) 
    {
    free(dyda);
    dyda=NULL;
    }
  
  if (!dyda) 
    {
    dyda=newitem(double,ma);
    dim_dyda = ma;
    }
  
  for (j=0;j<mfit;j++) 
    {
    for (k=0;k<=j;k++) alpha->data[j][k]=0.0;
    beta[j]=0.0;
    }
  
  *chisq=0.0;

  for (i=0;i<ndata;i++) 
    {
    sig2i = 1.0/sqr(sig[i]);
    ymod = (*funcs)(x[i],a,dyda,ma);
    dy=y[i]-ymod;
    (*chisq) += sqr(dy)*sig2i;
    for (j=0;j<mfit;j++) 
      {
      wt=dyda[lista[j]]*sig2i;
      beta[j] += dy*wt;
      for (k=0;k<=j;k++)
        alpha->data[j][k] += wt*dyda[lista[k]];
      }
    }
  
  for (j=1;j<mfit;j++)
    for (k=0;k<=j-1;k++) alpha->data[k][j]=alpha->data[j][k];
  }

/*******
mrqmin.c
*******/

/*EP*/ void mrqmin
  (
  double *x,
  double *y,
  double *sig,
  int ndata,
  double *a,
  int ma,
  int *lista,
  int mfit,
  d2matrix *covar,
  d2matrix *alpha,
  double *chisq,
  double (*funcs)(),
  double *alamda
  )
  {
  int k,kk,j,ihit;
  static double *da,*atry,*beta,ochisq;
  static d2matrix *oneda;
  static int almfit,alma;
    
  if (*alamda < 0.0) 
    {
    covar->nr = covar->nc = mfit; /* temporarily shrink covar */

    if (oneda && mfit!=almfit) 
      {
      free_d2matrix(oneda);
      oneda=NULL;
      }
    if (!oneda) 
      {
      oneda=new_d2matrix(mfit,1,1);
      almfit=mfit;
      }
    if (atry && ma!=alma)
      {
      free(atry); free(da); free(beta);
      atry=da=beta=NULL;
      }
    if (!atry)
      {
      atry=newitem(double,ma);
      da=newitem(double,ma);
      beta=newitem(double,ma);
      alma=ma;
      }
    
    kk=mfit;
    
    for (j=0;j<ma;j++) 
      {
      ihit=0;
      for (k=0;k<mfit;k++) if (lista[k] == j) ihit++;
      if (ihit == 0) lista[kk++]=j;
      else if (ihit > 1) nrerror("Bad LISTA permutation in MRQMIN-1");
      }
    if (kk != ma) nrerror("Bad LISTA permutation in MRQMIN-2");

    *alamda = 0.001;
    mrqcof(x,y,sig,ndata,a,ma,lista,mfit,alpha,beta,chisq,funcs);
    ochisq = *chisq;
    }
  
  for (j=0;j<mfit;j++) 
    {
    for (k=0;k<mfit;k++) covar->data[j][k]=alpha->data[j][k];
    covar->data[j][j]=alpha->data[j][j]*(1.0+(*alamda));
    oneda->data[j][0]=beta[j];
    }
  
  gaussj(covar,oneda);
  for (j=0;j<mfit;j++) da[j]=oneda->data[j][0];

  if (*alamda == 0.0) 
    {
    covar->nr = covar->nc = ma;  /* fix covar before it is meaningful */
    covsrt(covar,ma,lista,mfit);
    return;
    }
  
  for (j=0;j<ma;j++) atry[j]=a[j];
  for (j=0;j<mfit;j++) atry[lista[j]] = a[lista[j]]+da[j];
  
  mrqcof(x,y,sig,ndata,atry,ma,lista,mfit,covar,da,chisq,funcs);
  
  if (*chisq < ochisq) 
    {
    *alamda *= 0.1;
    ochisq=(*chisq);
    for (j=0;j<mfit;j++) 
      {
      for (k=0;k<mfit;k++) alpha->data[j][k]=covar->data[j][k];
      beta[j]=da[j];
      a[lista[j]]=atry[lista[j]];
      }
    } 
  else 
    {
    *alamda *= 10.0;
    *chisq=ochisq;
    }
  return;
  }

/*******
covsrt.c
*******/

/*EP*/ void covsrt(d2matrix *covar,int ma,int *lista,int mfit)
  {
  int i,j;
  double swap;

  for (i=1;i<ma;i++) 
    for (j=0;j<i;j++) covar->data[i][j]=0;  

  for (i=0;i<mfit-1;i++)
    for (j=i+1;j<mfit;j++) 
      {
      if (lista[j] > lista[i])
        covar->data[lista[j]][lista[i]]=covar->data[i][j];
      else
        covar->data[lista[i]][lista[j]]=covar->data[i][j];
      }
    
  swap=covar->data[0][0];
  for (j=0;j<ma;j++) 
    {
    covar->data[0][j]=covar->data[j][j];
    covar->data[j][j]=0.0;
    }
  
  covar->data[lista[0]][lista[0]]=swap;
  for (j=1;j<mfit;j++) covar->data[lista[j]][lista[j]]=covar->data[0][j];
  for (j=1;j<ma;j++)
    for (i=0;i<=j-1;i++) covar->data[i][j]=covar->data[j][i];
  }


/******
gausj.c
******/

#define SWAP(a,b) { temp=(a);(a)=(b);(b)=temp; }

/*EP*/ void gaussj(d2matrix *a,d2matrix *b)
  {
  double temp;
  int *indxc,*indxr,*ipiv;
  int i,icol,irow,j,k,l,ll,n,m;
  double big,dum,pivinv;

  n = a->nr;
  m = b->nc;
  
  if (n!=a->nc) nrerror("GAUSSJ: a not square matrix");
  if (n!=b->nr) nrerror("GAUSSJ: b has different number of rows than a");
  
  indxc=newitem(int,n);
  indxr=newitem(int,n);
  ipiv=newitem(int,n);
  
  for (j=0;j<n;j++) ipiv[j]=0;
  
  for (i=0;i<n;i++) 
    {
    big=0.0;
    for (j=0;j<n;j++)
      if (ipiv[j] != 1)
        for (k=0;k<n;k++) 
          {
          if (ipiv[k] == 0) 
            {
            if (fabs(a->data[j][k]) >= big) 
              {
              big=fabs(a->data[j][k]);
              irow=j;
              icol=k;
              }
            } else if (ipiv[k] > 1) nrerror("GAUSSJ: Singular Matrix-1");
          }
    ++(ipiv[icol]);
    if (irow != icol) 
      {
      for (l=0;l<n;l++) SWAP(a->data[irow][l],a->data[icol][l]);
      for (l=0;l<m;l++) SWAP(b->data[irow][l],b->data[icol][l]);
      }
    indxr[i]=irow;
    indxc[i]=icol;
    if (a->data[icol][icol] == 0.0) nrerror("GAUSSJ: Singular Matrix-2");
    pivinv=1.0/a->data[icol][icol];
    a->data[icol][icol]=1.0;
    for (l=0;l<n;l++) a->data[icol][l] *= pivinv;
    for (l=0;l<m;l++) b->data[icol][l] *= pivinv;
    for (ll=0;ll<n;ll++)
      if (ll != icol) 
        {
        dum=a->data[ll][icol];
        a->data[ll][icol]=0.0;
        for (l=0;l<n;l++) a->data[ll][l] -= a->data[icol][l]*dum;
        for (l=0;l<m;l++) b->data[ll][l] -= b->data[icol][l]*dum;
        }
    }
  for (l=n-1;l>=0;l--) 
    {
    if (indxr[l] != indxc[l])
      for (k=0;k<n;k++)
        SWAP(a->data[k][indxr[l]],a->data[k][indxc[l]]);
    }
  free(ipiv);
  free(indxr);
  free(indxc);
  }

#undef SWAP


#define TINY (1.0e-20)

/*EP*/ void ludcmp(d2matrix *a,int *indx,int *d)
  {
  int i,imax,j,k,n;
  double big,dum,sum,temp;
  double *vv;

  if (a->nr != a->nc) nrerror("mathpac: LUDCMP matrix not square\n");

  n = a->nr;
  vv=newitem_ni(double,n);
  
  *d=1;
  for (i=0;i<n;i++) 
    {
    big=0.0;
    for (j=0;j<n;j++) if ((temp=fabs(a->data[i][j])) > big) big=temp;
    if (big == 0.0) nrerror("Singular matrix in routine LUDCMP");
    vv[i]=1.0/big;
    }
  
  for (j=0;j<n;j++) 
    {
    for (i=0;i<j;i++) 
      {
      sum=a->data[i][j];
      for (k=0;k<i;k++) sum -= a->data[i][k]*a->data[k][j];
      a->data[i][j]=sum;
      }
    
    big=0.0;
    for (i=j;i<n;i++) 
      {
      sum=a->data[i][j];
      for (k=0;k<j;k++) sum -= a->data[i][k]*a->data[k][j];
      a->data[i][j]=sum;
      if ( (dum=vv[i]*fabs(sum)) >= big) 
        {
        big=dum;
        imax=i;
        }
      }
    
    if (j != imax) 
      {
      for (k=0;k<n;k++) 
        {
        dum=a->data[imax][k];
        a->data[imax][k]=a->data[j][k];
        a->data[j][k]=dum;
        }
      *d = -(*d);
      vv[imax]=vv[j];
      }
    indx[j]=imax;
    if (a->data[j][j] == 0.0) a->data[j][j]=TINY;
    if (j != n) 
      {
      dum=1.0/(a->data[j][j]);
      for (i=j+1;i<n;i++) a->data[i][j] *= dum;
      }
    }
    free(vv);
  }

#undef TINY

/*EP*/ void lubksb(d2matrix *a,int *indx,double *b)
  {
  int i,ii,ip,j,n;
  double sum;

  if (a->nr != a->nc) nrerror("mathpac: LUDCMP matrix not square\n");

  n = a->nr;
  ii= -1;
  for (i=0;i<n;i++) 
    {
    ip=indx[i];
    sum=b[ip];
    b[ip]=b[i];
    if (ii>=0) for (j=ii;j<=i-1;j++) sum -= a->data[i][j]*b[j];
    else if (sum) ii=i;
    b[i]=sum;
    }
  for (i=n-1;i>=0;i--) 
    {
    sum=b[i];
    for (j=i+1;j<n;j++) sum -= a->data[i][j]*b[j];
    b[i]=sum/a->data[i][i];
    }
  }


#ifndef MATHPAC
#define MATHPAC

#include "common.h"

typedef struct FCOMPLEX {double r,i;} fcomplex;
typedef struct IMMENSE {unsigned long l,r;} immense;
typedef struct GREAT {unsigned short l,c,r;} great;

typedef struct
  {
  int inext,inextp,iff,dummy;
  int ma[56];
  } ran3state ;

typedef struct
  {
  int iset,dummy;
  double gset;
  ran3state state;
  } gasdevstate;

typedef struct
  {
  int temp;
  int dimen;
  int nr,nc;
  double **data;
  } d2matrix;

void nrerror(char *error_text);
double cos2(double theta);
void allocd2matrixdata(d2matrix *rv,int zero);
d2matrix *new_d2matrix(int nr,int nc,int zero);
void free_d2matrixdata(d2matrix *rv);
void free_d2matrix(d2matrix *rv);
void fprint_d2matrix(FILE *f,d2matrix *m);
d2matrix *d2matrix_inverse(d2matrix *d,d2matrix *a);
d2matrix *identityd2matrix(int n);
d2matrix *fscan_d2matrix(FILE *f);
d2matrix *d2matrix_copy(d2matrix *m);
d2matrix *d2matrix_sum(int n,...);
d2matrix *d2matrix_sub(d2matrix *d,d2matrix *a,d2matrix *b);
d2matrix *d2matrix_scale(d2matrix *d,d2matrix *a,double c);
d2matrix *d2matrix_mult(d2matrix *d,d2matrix *a,d2matrix *b);
int d2matrix_cmp(d2matrix *a,d2matrix *b);
d2matrix *d2matrix_trans(d2matrix *d,d2matrix *a);
d2matrix *d2matrixdestprep(d2matrix *provided,int nr,int nc);
d2matrix *diagonald2matrix(int n,...);
d2matrix *constantd2matrix(int nr,int nc, ...);
double bico(long n,long k);
double factln(long n);
double gammln(double xx);
double bicoln(long n,long k);
double ran3(int *idum);
double ran3ns(int *idum,ran3state *state);
double ran1(int *idum);
double gasdev(int *idum);
double gasdevns(int *idum,gasdevstate *state);
void mrqcof
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
  );
void mrqmin
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
  );
void covsrt(d2matrix *covar,int ma,int *lista,int mfit);
void gaussj(d2matrix *a,d2matrix *b);
void ludcmp(d2matrix *a,int *indx,int *d);
void lubksb(d2matrix *a,int *indx,double *b);

#endif /*MATHPAC*/

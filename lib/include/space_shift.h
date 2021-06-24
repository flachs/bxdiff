#ifndef __space_shift_h_
#define __space_shift_h_


#ifdef __GNUC__
#define ILOG2(v) (63- __builtin_clzll(v))
#else
#define BLOGFORM(x,b,g,l)  ( ((x) & ~((1<<(b))-1)) ? (l) : (g) )
#define ILOG2(x) BLOGFORM(x,16,BLOGFORM(x,8,BLOGFORM(x,4,BLOGFORM(x,2,BLOGFORM(x,1,0,1),BLOGFORM(x,3,2,3)),BLOGFORM(x,6,BLOGFORM(x,5,4,5),BLOGFORM(x,7,6,7))),BLOGFORM(x,12,BLOGFORM(x,10,BLOGFORM(x,9,8,9),BLOGFORM(x,11,10,11)),BLOGFORM(x,14,BLOGFORM(x,13,12,13),BLOGFORM(x,15,14,15)))),BLOGFORM(x,24,BLOGFORM(x,20,BLOGFORM(x,18,BLOGFORM(x,17,16,17),BLOGFORM(x,19,18,19)),BLOGFORM(x,22,BLOGFORM(x,21,20,21),BLOGFORM(x,23,22,23))),BLOGFORM(x,28,BLOGFORM(x,26,BLOGFORM(x,25,24,25),BLOGFORM(x,27,26,27)),BLOGFORM(x,30,BLOGFORM(x,29,28,29), ((x)&0x80000000?31:30) ))))
#endif

#define SPACE_SHIFT(v) ((v<=1) ? 0 : 1+ILOG2(v-1))

#endif

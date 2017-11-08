/* Defininitionen und Makros fuer Bitfelder.
 * Ein Bitfeld ist in dieser Version einfach vom Typ unsigned int.
 * Bit(feld)operationen sind also elementar.
 *
 * (c) Guido Gronek, Lion Ges. f. Systementwicklung m.b.H. & c't 5/95
 */

typedef unsigned BITFLD;

#define BITFLD_MAX      31

#define BITCLR(B,n)     ( (B) &= ~(1L<<(n)) )
#define BITSET(B,n)     ( (B) |= (1L<<(n)) )
#define BITISCLR(B,n)   ( ((B) & (1L<<(n)))==0 )
#define BITISSET(B,n)   ( ((B) & (1L<<(n))) )

#define BFRSHIFT(B,n)   ( (B) >> n )
#define BFLSHIFT(B,n)   ( (B) << n )
#define BFALL()         ( (BITFLD)~0L )
#define BFNULL()        ( 0L )
#define BFOR(B1,B2)     ( (B1) | (B2) )
#define BFAND(B1,B2)    ( (B1) & (B2) )

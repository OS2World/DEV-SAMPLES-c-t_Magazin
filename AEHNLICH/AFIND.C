/* afind.c: Approximative Suche mit dem Shift-AND-Algotithmus
 * (c) Guido Gronek, Lion Ges. f. Systementwicklung m.b.H. & c't 5/95
 */
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "bitfld.h"
#include "parser.h"
#include "afind.h"

static BITFLD MATCH,                         /* Matching Maske */
		  P,     /* 1-bit fuer fehlerfreie Musterteile */
	       STAR,     /* 0-bit hinter *-Wildcards im Muster */
		  N,   /* 0-bit, wenn erstes Zeichen im Muster */
     T[UCHAR_MAX+1],        /* charakt. Vektoren aller Zeichen */
     TT[UCHAR_MAX+1];     /* ... modifiziert fuer Multipattern */

static int M;                                  /* Musterlaenge */
static int Withstars;             /* flag: Muster enthaelt '*' */
static int Laststar;    /* flag: letztes Musterzeichen ist '*' */
static int Leftstar;    /* Position-1 des ersten '*' im Muster */
static int Ignore;       /* flag: Gross- gleich Kleinbuchstabe */

int amatcherr;                  /* Zahl der Fehler des Matches */

#define cap(c)     ( islower(c) ? toupper(c) : tolower(c) )
#define strlast(t) ( t + strlen(t) - 1 )

/* user() wird vom Parser aufgerufen, wenn entweder ein Zeichen,
 * eine Zeichenklasse, ein Wildcard oder die Token vom Typ
 * T_LBRAC, T_RBRAC oder T_OR erkannt wurden.
 */
int user(void)
{ static int index;      /* Nummer des akt. Zeichens im Muster */
  BITFLD *TC;
  int c,j;

  if ( M < 0 ) {                              /* erster Aufruf */
    for ( c=UCHAR_MAX+1, TC=T; --c>=0; ++TC )
      *TC = BFALL();
    MATCH = STAR = BFALL();
    N = BFALL(); BITCLR(N,0); P = BFNULL();
    Withstars = 0; Leftstar = -2;
    M = 0; index = 0;
  }
  j = index;
  if ( M > BITFLD_MAX ) return 1;       /* Muster ist zu lang! */
  switch ( Token.type ) {
    case T_STAR:                               /* '*'-Wildcard */
      if ( Lasttoken.type == T_STAR || M==0 )
	return 0;            /* am Anfang und vor * ignorieren */
      if ( Withstars == 0 ) {
	Withstars = 1;
	Leftstar = j;                     /* erstes '*' merken */
      }
      Laststar = j;
      if ( j>=0 && j<=BITFLD_MAX)
	BITCLR(STAR,j);
    break;
    case T_QM:                                 /* '?'-Wildcard */
      memset(Cset,1,sizeof(Cset));  /* steht fuer alle Zeichen */
    case T_RPAREN:                            /* Zeichenklasse */
      Laststar = 0;
      for ( c=UCHAR_MAX+1; --c>0;  )
	if ( Cset[c] ) {           /* wenn Zeichen i in Klasse */
	  BITCLR( *(T+c) ,j );            /* dann Bit loeschen */
	  if ( Ignore )              /* case-insensitive Suche */
	    BITCLR( *(T+cap(c)) ,j );
	}
      if (Protected) BITSET(P,j);
      ++j; ++M;
    break;
    case T_LBRAC:               /* fehlerfreier Bereich Anfang */
      if ( Protected ) return 1;                   /* Fehler ! */
      else Protected = 1;
    break;
    case T_RBRAC:                 /* fehlerfreier Bereich Ende */
      if ( Protected ) Protected = 0;
      else return 1;                               /* Fehler ! */
    break;
    case T_OR:                         /* neues Parallelmuster */
      if ( Protected ) return 1;                   /* Fehler ! */
      BITCLR(N,j);
    break;
    default:                                    /* ein Zeichen */
      Laststar = 0;                   /* ist kein '*'-Wildcard */
      c = Token.value;
      BITCLR( *(T + c) ,j );
      if ( Ignore )                  /* case-insensitive Suche */
	BITCLR( *(T + cap(c)) ,j );
      if ( Protected ) BITSET(P,j);
      ++j; ++M;
    break;
  }
  index = j;
  return 0;
}

/* init_afind() bereitet die Suche vor
 * Ergebnis: 0, falls alles O.K.
 *           1, falls das Muster zu lang oder fehlerhaft ist
 */
int init_afind(char **pattern, int ignore)
{ int c,state;

  M = -1;
  Ignore = ignore;
  state = parsepattern(pattern);
  Laststar = (Laststar==M-1);
  for ( c=UCHAR_MAX+1; --c>=0; )
    TT[c] = BFOR(T[c],N);    /* modifizierte charakt. Vektoren */
  MATCH = BFRSHIFT(N,1);  /* Matching-Maske mit 0-bits, wo ein */
  BITCLR(MATCH,M-1);    /* .. Zeichen das letzte im Muster ist */
  return (M > BITFLD_MAX) ? 1 : state;
}

/* afind:   durchsucht Text nach dem vorbereiteten Muster
 * Eingabe: text:     Suchtext
 *          errors:   Maximal zugelassene Fehler
 *          minimize: flag: Match mit moeglichst wenig Fehlern
 * Ausgabe: Zeiger auf Textstelle am Ende des Matches, oder NULL
 *          die globale Variable amatcherr enthaelt die Zahl
 *          der Fehler beim Match
 */
char *afind(char *text, int errors, int minimize)
{ static BITFLD STATES[MISMATCH_MAX+2];
  BITFLD S, SS, SOR, TC, *SP;
  char *t, *match = NULL;
  int d;

  if ( text ==NULL ) return NULL;
  amatcherr = 0;
  SS = BFALL();                            /* SS ist S<j><d-1> */
  SP = STATES;           /* Initialisiere die Zustandsvektoren */
  for ( d=errors+1; --d>=0; SP[d] = SS ) ;
  for ( t=text; *t; ++t ) {          /* fuer jedes Textzeichen */
    TC = *(T + *t);        /* charakt. Vektor fuer das Zeichen */
    for ( d=0; d<=errors; ++d )    /* fuer jeden moegl. Fehler */
    { S = SP[d];                              /* S ist S<j><d> */
      SOR = BFOR(BFLSHIFT(S,1),TC);
      SOR = BFAND(SOR,TT[*t]);
      if ( d>0 ) {
	SS = BFAND( SP[d-1], BFLSHIFT(BFAND(SS,SP[d-1]),1) );
	SS = BFAND( BFOR(P,SS), SOR );
      }
      else SS = SOR;
      if (Withstars) SS = BFAND(SS,BFOR(S,STAR));
      if ( BFOR(SS,MATCH) != BFALL() ) { /* Match mit d Fehlern */
	amatcherr = d;
	if ( d==0 || !minimize ) return t;
	match = t; errors = d-1;
      }
      SP[d] = SS;
      SS = S;
    }
  }
  return match;
}

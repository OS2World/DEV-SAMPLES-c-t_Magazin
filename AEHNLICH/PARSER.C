/* parser.c: Muster-Parser als "Finite State Machine"
 *
 * (c) Guido Gronek, Lion Ges. f. Systementwicklung m.b.H. & c't 5/95
 */
#include <limits.h>
#include <string.h>
#include "parser.h"

static int init(), copen(), cinvrt(), cadd(), cminus();

int (*statefunc[]) () = {       /* Zustandsfunktionen der FSM */
init,   user,   copen,  cinvrt, cadd,   cminus};

enum states {                        /* alle Zustaende der FSM */
INIT, USER,  COPEN, CINVRT, CADD,  CMINUS,
ERR, END};

int transtable[] = {              /* Zustandsuebergangstabelle */
USER, USER,  CADD,  CADD,   CADD,  CADD,       /* T_CHAR */
USER, USER,  CADD,  CADD,   CADD,  CADD,         /* T_QM */
USER, USER,  CADD,  CADD,   CADD,  CADD,        /* T_STAR */
COPEN,COPEN, ERR,   ERR,    ERR,   ERR,       /* T_LPAREN */
USER, USER,  CINVRT,CADD,   CADD,  CADD,      /* T_CINVRT */
USER, USER,  ERR,   ERR,    CMINUS,CADD,       /* T_MINUS */
ERR,  ERR,   ERR,   ERR,    USER,  ERR,       /* T_RPAREN */
END,  END,   ERR,   ERR,    ERR,   ERR,          /* T_EOT */
ERR,  ERR,   ERR,   ERR,    ERR,   ERR,         /* T_ILL */
USER, USER,  CADD,  CADD,   CADD,  CADD,      /* T_LBRAC */
ERR,  USER,  CADD,  CADD,   CADD,  CADD,      /* T_RBRAC */
ERR,  USER,  CADD,  CADD,   CADD,  CADD,      /* T_OR */
};

struct token_t Token, Lasttoken;
char *Pattern;
charset_t Cset;
int Protected;
static rangefrom, csetfill;

/* gettoken() holt das naechstes Token
 * Ergebnis: der Type des Token
  */
int gettoken(char **pattern)
{ int c, t = T_CHAR;
  char *p = *pattern;

  Lasttoken = Token;
  if ( (c = *p++) == '\\' ) {   /* durch Backslash geschuetzt */
    if ( (c = *p++) == 0 )      /* Backslash am Ende */
      t = T_ILL;                /* nicht erlaubt ! */
  }
  else {                      /* nicht geschuetztes Zeichen */
    switch ( c ) {
      case '?' : t = T_QM; break;
      case '*' : t = T_STAR; break;
      case '[' : t = T_LPAREN; break;
      case ']' : t = T_RPAREN; break;
      case '-' : t = T_MINUS; break;
      case '{' : t = T_LBRAC; break;
      case '}' : t = T_RBRAC; break;
      case '|' : t = T_OR; break;
      case '!' :
      case '^' : t = T_CINVRT; break;
      case 0   : t = T_EOT;
    }
  }
  *pattern = p;
  return Token.value = c, Token.type = t;
}

int init(void)                      /* initialisiert den Parser */
{ memset(&Token,0,sizeof(Token));
  return Protected = 0;
}

int copen(void)                  /* oeffnet eine Zeichenklasse */
{ memset(&Cset,0,sizeof(Cset));
  csetfill = 1;
  return rangefrom = 0;
}

int cinvrt(void)              /* erzeugt eine Komplement-Klasse */
{ memset(&Cset,1,sizeof(Cset));
  return csetfill = 0;
}

int cminus(void)   /* merkt sich Anfang eines Zeichenbereiches */
{ rangefrom = Lasttoken.value;
  return 0;
}

int cadd(void)                  /* fuegt Zeichen in Klasse ein */
{ int c, rangeto = Token.value;

  if ( !rangefrom ) rangefrom = rangeto;
  if ( rangefrom > rangeto ) {
    c = rangefrom; rangefrom = rangeto; rangeto = c;
  }
  for ( c = rangefrom; c <= rangeto; ++c )
    Cset[c] = csetfill;
  return rangefrom = 0;
}

/* parsepattern():
 * Durchlaueft in einer Schleife verschiedene Zustaende, bis ein
 * Endzustand erreicht ist. Der jeweils naechste Zustand ergibt
 * sich mit Hilfe einer Uebergangstabelle aus dem Zustand selbst
 * und dem naechsten Token. Fuer jeden Zustand wird die passende
 * Zustandsfunktion geholt und ausfuehrt.
 */
int parsepattern(char **pattern)
{ int state = INIT;                         /* Ausgangszustand */
  Pattern = *pattern;

  do {
    if ( statefunc[state]() )            /* Funktion ausfuehren */
      state = ERR;
    else
      state = transtable[state+gettoken(&Pattern)];
  } while ( state < ERR );
  if ( Protected || Token.type==T_OR ) state = ERR;
  return *pattern = Pattern-1 , (state  - END);
}

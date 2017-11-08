/* parser.h: Headerfile fuer den Muster-Parser
 * Aufruf:   int state;
 *           state = parsepattern(char **pattern);
 * Ergebnis: 0, falls das Muster einwandfrei ist
 *           im Fehlerfall ist das Ergebnis ungleich 0 und
 *           pattern zeigt auf die fehlerhafte Stelle im Muster
 * Beschreibung:
 *   parsepattern() betrachtet das Muster als Folge von Token
 *   und erkennt die folgenden Typen von Token:
 *                   c : ein normales Zeichen         (T_CHAR)
 *                  \c : mit '\' geschuetztes Zeichen (T_CHAR)
 *             [abc-h] : Zeichenklasse                (T_RPAREN)
 *   [^abc] bzw [!abc] : Komplement-Zeichenklasse     (T_RPAREN)
 *                   ? : ein beliebiges Zeichen       (T_QM)
 *                   * : beliebig viele Zeichen       (T_STAR)
 *               {abc} : zwischen {} Fehler unerlaubt
 *
 * (c) Guido Gronek, Lion Ges. f. Systementwicklung m.b.H. & c't 5/95
 */

#define char unsigned char
#define NSTATES     6  /* Zahl d. nichtterminalen Zustaende */

enum tokentype {               /* die erkannten Token */
T_CHAR,             T_QM=NSTATES,       T_STAR=2*NSTATES,
T_LPAREN=3*NSTATES, T_CINVRT=4*NSTATES, T_MINUS=5*NSTATES,
T_RPAREN=6*NSTATES, T_EOT=7*NSTATES,    T_ILL=8*NSTATES,
T_LBRAC=9*NSTATES,  T_RBRAC=10*NSTATES, T_OR=11*NSTATES };

typedef struct token_t {             /* ein Token */
  int  type;                         /* dessen Typ */
  int value;                         /* dessen Wert */
} TOKEN_T;

typedef char charset_t[UCHAR_MAX+1];

int parsepattern(char **pattern);
extern int user();

extern TOKEN_T Token, Lasttoken;     /* akt. und voriges Token */
extern charset_t Cset;                   /* akt. Zeichenklasse */
extern int Csetinv;           /* flag: Z.klasse ist invertiert */
extern int State;
extern int Protected;
extern char *Pattern;

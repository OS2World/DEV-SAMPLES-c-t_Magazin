/* afind.h: Headerfile fuer amatch.c */
/* (c) Guido Gronek, Lion Ges. f. Systementwicklung m.b.H. & c't 5/95 */

#define MISMATCH_MAX  8

extern int amatchlen;
extern int amatcherr;

int init_afind(char **pattern, int ignore);
char *afind(char *text, int errors, int minimize);

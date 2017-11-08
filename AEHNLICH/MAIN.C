/* main.c: Testtreiber fuer approximatives Matching
 *         durchsucht eine Datei, oder die Standardeingabe,
 *         nach dem angegebenen Muster
 * Aufruf: afind [-Fehler] [-i] [-b] [-e] Muster [Datei]
 * Optionen: -Fehler: maximal <Fehler> erlaubt
 *           -i : ignoriert Gross- und Kleinschreibung
 *           -b : sucht den besten Match in einer Zeile
 *           -e : gibt auch die Fehlerzahl aus
 *
 * (c) Guido Gronek, Lion Ges. f. Systementwicklung m.b.H. & c't 5/95
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include "parser.h"
#include "afind.h"

#define USAGE "Aufruf: afind [-Fehler] [-i] [-b] [-e] Muster [Datei]"
#define ERR(msg) {fprintf(stderr,msg); fprintf(stderr,"\n"); exit(1);}

char  *optarg;  /* Zeiger auf Argument */
int optind = 0; /* argv Index der Option */

/* getopt: holt naechste Option
 * Eingabe:  argc, argv : Agumentvektor und -zaehler
 *           optstring  : String mit allen Optionszeichen
 *                        "c:" ist Option "c" mit Argument
 * Ergebnis: das Optionszeichen oder '?' bei illegaler Option
 *           die glob. Variablen optind und optarg liefern den
 *           argv-Index und ggf. das Argument der Option
 */
int getopt(int argc, char *argv[], char *optstring)
{ static char *scan;                      /* laeuft ueber argv */
  char c, *p;                      /* p laeuft ueber optstring */

  optarg = NULL;
  if ( optind == 0 ) {                        /* erster Aufruf */
    scan = NULL;
    optind++;
  }
  if ( scan == NULL || *scan == 0 ) {       /* naechste Option */
    if ( optind >= argc || argv[optind][0] != '-' )
      return EOF;          /* Ende der Optionen oder Argumente */
    scan = argv[optind]+1;
    optind++;
  }
  c = *scan++;                                   /* die Option */
  for ( p = optstring; *p != 0 && *p != c; ++p ); /* .. suchen */
  if ( *p == 0 ) return '?';                 /* nicht gefunden */
  if ( *++p == ':' ) {          /* Option besitzt ein Argument */
    if ( *scan != 0 ) {
      optarg = scan;                  /* Argument folgt direkt */
      scan = NULL;
    }
    else if ( optind >= argc )         /* war letztes Argument */
      return '?';
    else {              /* Argument ist naechstes argv Element */
      optarg = argv[optind];
      optind++;
    }
  }
  return c & 0xff;
}

int main( argc, argv )
int  argc;
char *argv[];
{ int c;
  int ignore = 0, perrors = 0, bestmatch = 0;         /* flags */
  int errors = 0;
  FILE *stream;
  char *p, *t;
  static char buffer[256];       /* Max. 256 Zeichen pro Zeile */
                                 /* im Text!                   */
  while ( (c = getopt(argc,argv,"012345678ibe")) != EOF ) {
    switch(c) {
      case 'b': bestmatch = 1; break;
      case 'i': ignore = 1; break;
      case '0': case '1': case '2': case '3':
      case '4': case '5': case '6': case '7':
      case '8': errors = c-'0'; break;
      case 'e': perrors = 1; break;
      case '?': ERR(USAGE);
    }
  }
  argc -= optind;               /* noch verbleibende Argumente */
  argv += optind;                      /* .. und Zeiger darauf */
  if ( argc ==0 ) ERR(USAGE);
  p = *argv++;                                   /* das Muster */
  if ( init_afind(&p,ignore) != 0 )          /* ..vorbereiten */
    ERR("Fehler im Muster!");
  if ( --argc == 0 )            /* falls keine Datei angegeben */
    stream = stdin;                    /* dann von stdin lesen */
  else if ( (stream = fopen(*argv,"r")) ==NULL )
    ERR("Kann Textdatei nicht oeffnen!");
  do {         /* Textzeilen lesen und nach Muster durchsuchen */
    t = buffer;
    while ( (c = getc(stream)) != EOF )
      if ( c != '\n' ) *t++ = c;
        else break;
    *t = 0;
    if ( afind(buffer,errors,bestmatch) ) {       /* gefunden! */
      if ( stream != stdin ) printf("%s: ",*argv);
      if (perrors && amatcherr)
        printf("(%d): ", amatcherr);
      printf("%s\n",buffer);
    }
  } while ( c != EOF );
  if ( stream != stdin ) fclose(stream);
  return 0;
}

/*
 * Definitions etc. for regexp routines.
 *
 */
#define NSUBEXP  10
typedef struct regexp {
        char *startp[NSUBEXP];
        char *endp[NSUBEXP];
        char regstart;          /* Internal use only. */
        char reganch;           /* Internal use only. */
        char *regmust;          /* Internal use only. */
        int regmlen;            /* Internal use only. */
        char program[1];        /* Unwarranted chumminess with compiler. */
} regexp;

regexp * __cdecl regcomp(char *exp);
int __cdecl regexec(regexp *prog,char * string);
void __cdecl regsub(regexp *prog,char *source,char *dest);
void __cdecl regerror(char *s);


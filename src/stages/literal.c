/*
 *
 *        Name: literal.c (C program source)
 *              Ductwork LITERAL stage
 *        Date: 2023-06-09 (Friday) Gallatin
 *              2023-06-27 (Tuesday) Belle Pre
 *              sends the argument string to next stage via xploutput()
 *              and then passes all input records to the output
 *              looping over xplpeekto(), xploutput(), xplreadto(),
 *              to not delay records
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <xpllib.h>

/* ------------------------------------------------------------------ */
int main(int argc,char*argv[])
  {
    static char _eyecatcher[] = "pipeline stage 'literal' main()";

    int i, buflen, rc;
    char buffer[4096], *p, *q;
    struct PIPECONN *pc, *pi, *po, *pn;

printf("literal: (starting)\n");

    /* NOTE: standard operation is for the dispatcher                 *
     * to provide all arguments as a single string in argv[1].        */

    /* initialize this stage                                          */
    rc = xplstagestart(&pc);
    if (rc < 0) return 1;

    /* snag the first input stream and the first output stream        */
    pi = po = NULL;
    for (pn = pc; pn != NULL; pn = pn->next)
      { if (pn->flag & XPL_OUTPUT) { if (po == NULL) po = pn; }
        if (pn->flag & XPL_INPUT)  { if (pi == NULL) pi = pn; } }

    /* FIXME: provide an error message "no output stream */
    if (po == NULL) return 1;
printf("literal: (YES output is connected)\n");

//printf("literal: %s\n",argv[1]);
    /* write the literal string to our primary output stream          */
    if (argc > 1 && *argv[1] != 0x00)
      {
//      p = argv[1];
//p = "this R a test";
p = xplcatargs(argc,argv);
        rc = xploutput(po,p,strlen(p));
        if (rc < 0) return 1;
      }
printf("literal: %s (sent the record)\n",argv[1]);

    /* once we have written the literal to the output stream
     * we then copy all input records, if any, to output              */
//  if (pi == NULL) return 0;
//printf("literal: (we seem to also have an input stream)\n");

    if (pi != NULL)
    while (1) {
        buflen = sizeof(buffer) - 1;
        rc = xplpeekto(pi,buffer,buflen);             /* sip on input */
        if (rc < 0) break; /* else */ buflen = rc;
        rc = xploutput(po,buffer,buflen);       /* send it downstream */
        if (rc < 0) break;
        xplreadto(pi,NULL,0);     /* consume the record after sending */
      }
    if (rc < 0) return 1;

    /* terminate this stage cleanly                                   */
    rc = xplstagequit(pc);
    if (rc < 0) return 1;

printf("literal: (normal exit)\n");

    return 0;
  }



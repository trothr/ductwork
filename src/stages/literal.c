/*
 *
 *        Name: literal.c (C program source)
 *              Ductwork LITERAL stage
 *              sends the argument string to next stage via fpxoutput()
 *              and then passes all input records to the output
 *              fpxpeekto(), fpxoutput(), fpxreadto(), not delay record
 *        Date: 2023-06-09 (Friday) Gallatin
 *              2023-06-27 (Tuesday) Belle Pre
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <fpxlib.h>

/* for development */
#include "fpxlib.c"

int main(int argc,char*argv[])
  {
    static char _eyecatcher[] = "pipeline stage 'literal' main()";

    int i, buflen, rc;
    char buffer[4096], *p, *q;
    struct PIPECONN *pc, *pi, *po, *pn;


    /* NOTE: standard operation is for the dispatcher                 *
     * to provide all arguments as a single string in argv[1].        */

    /* initialize this stage                                          */
    rc = fpxstagestart(&pc);
    if (rc < 0) return 1;


    /* snag the first input stream and the first output stream        */
    pi = po = NULL;
    for (pn = pc; pn != NULL; pn = pn->next)
      { if (pn->side) { if (po == NULL) po = pn; }          /* output */
                 else { if (pi == NULL) pi = pn; } }         /* input */

    /* write the literal string to our primary output asa record      */
    if (argc > 1 && *argv[1] != 0x00)
if (po != NULL)
      {
    rc = fpxoutput(po,argv[1],strlen(argv[1]));
    if (rc < 0) return 1;
      }


    while (1) {
        buflen = sizeof(buffer) - 1;
        rc = fpxpeekto(pi,buffer,buflen);             /* sip on input */
        if (rc < 0) break; /* else */ buflen = rc;
        rc = fpxoutput(po,buffer,buflen);       /* send it downstream */
        if (rc < 0) break;
        fpxreadto(pi,NULL,0);     /* consume the record after sending */
      }
    if (rc < 0) return 1;

    return 0;
  }



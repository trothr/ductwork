/*
 *
 *        Name: fanin.c (C program source)
 *              Ductwork FANIN stage
 *        Date: 2024-05-05
 *              reads all input records, holds them in memory,
 *              then writes them all out after input is severed
 *
 * From the help for CMS Pipelines 'FANIN',
   "fanin" passes all records on the primary input stream to the
   primary output stream, then all records on the secondary input
   stream to the primary output stream, and so on.
 */

#include <stdio.h>

#include <xfl.h>

/* ------------------------------------------------------------------ */
int main()
  {
    static char _eyecatcher[] = "pipeline stage 'fanin' main()";

    int i, buflen, rc;
    char buffer[4096], *p, *q, *args;
    struct PIPECONN *pc, *pi, *po, *pn;

printf("fanin: (starting)\n");

    /* initialize this stage                                          */
    rc = xfl_stagestart(&pc);
    if (rc < 0) return 1;

    /* snag the fprimary output stream (scanning all connectors)      */
    po = NULL;
    for (pn = pc; po == NULL && pn != NULL; pn = pn->next)
      if (pn->flag & XFL_OUTPUT) po = pn;

    /* if no output then stop now */
    if (po == NULL)
      {
        xfl_stagequit(pc);
        return 0;
      }
/* CMS Pipelines 'fanin' does not complain about this condition */
printf("fanin: YES output is connected\n");

    /* snag the first input stream from the chain-o-connectors        */
    pi = NULL;
    for (pn = pc; pi == NULL && pn != NULL; pn = pn->next)
      if (pn->flag & XFL_INPUT)  pi = pn;

if (pi != NULL)
printf("fanin: YES input is connected\n");

    while (pi != NULL)
      {
        /* "Do Forever" until we break out otherwise */
        while (1)
          {
            /* perform a PEEKTO and see if there is a record ready        */
            buflen = sizeof(buffer) - 1;
            rc = xfl_peekto(pi,buffer,buflen);            /* sip on input */
            if (rc < 0) break; /* else */ buflen = rc;
printf("fanin: got a record\n");

            /* write the record to our primary output stream              */
            rc = xfl_output(po,buffer,buflen);      /* send it downstream */
            if (rc < 0) break;
printf("fanin: sent that record downstream\n");

            /* now consume the record from the input stream               */
            rc = xfl_readto(pi,NULL,0);   /* consume record after sending */
            if (rc < 0) break;
printf("fanin: consumed the record\n");
          }

        /* snag the next input, if any */
        for (pn = pi; pn != NULL; pn = pn->next)
          if (pn->flag & XFL_INPUT) { pi = pn; break; }
      }

    /* terminate this stage cleanly                                   */
    rc = xfl_stagequit(pc);
    if (rc < 0) return 1;

printf("fanin: (normal exit)\n");

    /* pass a "success" return code to our caller */
    return 0;
  }



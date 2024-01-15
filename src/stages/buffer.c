/*
 *
 *        Name: buffer.c (C program source)
 *              Ductwork BUFFER stage
 *        Date: 2009-Mar-05
 *              reads all input records, holds them in memory,
 *              then writes them all out after input is severed
 *
 */

#include <stdio.h>

#include <xfl.h>

/* ------------------------------------------------------------------ */
int main()
  {
    static char _eyecatcher[] = "pipeline stage 'buffer' main()";

//  struct  recinmem { void  *addr;  int size; };
    int i, buflen, rc;
    char buffer[4096], *p, *q, *args;
    struct PIPECONN *pc, *pi, *po, *pn;

//printf("buffer: (starting)\n");

    /* initialize this stage                                          */
    rc = xfl_stagestart(&pc);
    if (rc < 0) return 1;

    /* snag the first input stream and the first output stream        */
    pi = po = NULL;
    for (pn = pc; pn != NULL; pn = pn->next)
      { if (pn->flag & XFL_OUTPUT) { if (po == NULL) po = pn; }
        if (pn->flag & XFL_INPUT)  { if (pi == NULL) pi = pn; } }

    /* FIXME: provide an error message "no output stream */
    if (pi == NULL)
      { xfl_error(61,0,NULL,"BUF");        /* provide specific report */
        return 1; }
//printf("buffer: YES input is connected\n");
    if (po == NULL)
      { xfl_error(61,0,NULL,"BUF");        /* provide specific report */
        return 1; }
//printf("buffer: YES output is connected\n");

    /* start with an index offset of zero */
//  i = 0;

    /* allocate a chunk of memory to hold the record */
//  recinmem.addr[i] = ... some address or offset

    /* "Do Forever" until we break out otherwise */
    while (1) {
        /* allocate memory for that and loop */
//      recinmem.addr[i] = malloc();

        /* perform a PEEKTO and see if there is a record ready        */
        buflen = sizeof(buffer) - 1;
        rc = xfl_peekto(pi,buffer,buflen);            /* sip on input */
        if (rc < 0) break; /* else */ buflen = rc;
//printf("buffer: got a record\n");

        /* write the record to our primary output stream              */
        rc = xfl_output(po,buffer,buflen);      /* send it downstream */
        if (rc < 0) break;
//printf("buffer: sent that record downstream\n");

        /* now consume the record from the input stream               */
        rc = xfl_readto(pi,NULL,0);   /* consume record after sending */
        if (rc < 0) break;
//printf("buffer: consumed the record\n");

        /* bump up the index to the next record */
        i++;

      }

    /* last record allocated was not actually read so let it go */
//  (void) free(recinmem.addr[i]);  i--;

    /* loop through the records we have accumulated */
//  for (j = 0 ; j < i ; j++) {

        /* write a record to the output stream */
//      rc = output(PO0,recinmem.addr[j],recinmem.size[j]);

        /* and free the memory for that record */
//      (void) free(recinmem.addr[j]);
//    }

    /* terminate this stage cleanly                                   */
    rc = xfl_stagequit(pc);
    if (rc < 0) return 1;

//printf("buffer: (normal exit)\n");

    /* pass a "success" return code to our caller */
    return 0;
  }



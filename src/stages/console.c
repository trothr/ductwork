/*
 *
 *        Name: console.c (C program source)
 *              Ductwork CONSOLE stage
 *        Date: 2010-Mar-24
 *              2023-07
 *
 */

#include <stdio.h>


#include <xpllib.h>

/* ------------------------------------------------------------------ */
int main(int argc,char*argv[])
  {
    static char _eyecatcher[] = "pipeline stage 'console' main()";

    int i, buflen, rc;
    char buffer[4096], *p, *q;
    struct PIPECONN *pc, *pi, *po, *pn;

printf("console: (starting)\n");

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

    /* if pi is null then we are a first stage so slurp stdin         */
    if (pi == NULL && po == NULL) return 1;
    /* FIXME: provide an error message "no streams connected" */

    while (1) {
printf("console: (top of loop)\n");
        buflen = sizeof(buffer) - 1;
        rc = xplpeekto(pi,buffer,buflen);             /* sip on input */
        if (rc < 0) break; /* else */ buflen = rc;
printf("console: '%s' %d (input peeked)\n",buffer,rc);
printf("\n%s\n\n",buffer);
        if (po != NULL)
        rc = xploutput(po,buffer,buflen);       /* send it downstream */
        if (rc < 0) break;
        xplreadto(pi,NULL,0);     /* consume the record after sending */
printf("console: (record consumed)\n");
      }
    if (rc < 0) return 1;

    /* terminate this stage cleanly                                   */
    rc = xplstagequit(pc);
    if (rc < 0) return 1;

printf("console: (normal exit)\n");

    return 0;
  }


#ifdef OLDSTUFF

#include <ductwork.h>

int s_console_fd0();
int s_console_fd1();
struct PIPECONN PI0, PO0;      /* side, ctrl, data, buff, glob, recno */

/* ------------------------------------------------------------------ */
int oldmain()
  {
    /* firstly, are we a first stage? */
    switch (pipeline_streamstate(&PI0))
      {
        case 12:  /* with no primary input, we ARE a first stage */
        return s_console_fd0();

        default:  /* otherwise, we are an end or intermediate */
        return s_console_fd1();
      }
  }

/* * * * */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#endif



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

#include <xfl.h>

/* ------------------------------------------------------------------ */
int main(int argc,char*argv[])
  {
    static char _eyecatcher[] = "pipeline stage 'literal' main()";

    int i, buflen, rc, argl;
    char buffer[4096], *p, *q, *args;
    struct PIPECONN *pc, *pi, *po, *pn;

//printf("literal: (starting)\n");

/*
 * init/start                                                      works
 * get args                                                        works
 * write string to primary output
 * quit
 */

    /* initialize this stage                                          */
    rc = xfl_stagestart(&pc);
    if (rc < 0) return 1;

    /* string-up the command line arguments                           */
    args = xfl_argcat(argc,argv);
    if (args == NULL) return 1;

//printf("literal: '%s'\n",args);

    /* snag the first input stream and the first output stream        */
    pi = po = NULL;
    for (pn = pc; pn != NULL; pn = pn->next)
      { if (pn->flag & XFL_OUTPUT) { if (po == NULL) po = pn; }
        if (pn->flag & XFL_INPUT)  { if (pi == NULL) pi = pn; } }

    /* FIXME: provide an error message "no output stream */
    if (po == NULL)
      {
        xfl_error(61,0,NULL,"LIT");        /* provide specific report */
        return 1;
      }
//printf("literal: YES output is connected\n");

    /* write the literal string to our primary output stream          */
    argl = strlen(args);
    rc = xfl_output(po,args,argl);
    if (rc < 0) return 1;
//printf("literal: %s (sent the record)\n",argv[1]);


    /* proper pipeline: once we have written the literal to the       *
     * output, we then copy all input records, if any, to the output  */
#ifdef PROPER_PIPELINE
    if (pi != NULL)
//printf("literal: (we seem to also have an input stream)\n");
    while (1) {
        buflen = sizeof(buffer) - 1;
        rc = xplpeekto(pi,buffer,buflen);             /* sip on input */
        if (rc < 0) break; /* else */ buflen = rc;
        rc = xploutput(po,buffer,buflen);       /* send it downstream */
        if (rc < 0) break;
        xplreadto(pi,NULL,0);     /* consume the record after sending */
      }
    if (rc < 0) return 1;
#endif


    /* terminate this stage cleanly                                   */
    rc = xfl_stagequit(pc);
    if (rc < 0) return 1;

//printf("literal: (normal exit)\n");

    return 0;
  }



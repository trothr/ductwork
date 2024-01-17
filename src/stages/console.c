/*
 *
 *        Name: console.c (C program source)
 *              Ductwork CONSOLE stage
 *        Date: 2010-Mar-24, 2023-07, 2024
 *              reads stdin and writes to primary output
 *               -or-
 *              reads primary input and writes to stdout
 *
 */

#include <stdio.h>

#include <xfl.h>

/* ------------------------------------------------------------------ */
int main(int argc,char*argv[])
  {
    static char _eyecatcher[] = "pipeline stage 'console' main()";

    int i, buflen, rc;
    char buffer[4096], *p, *q;
    struct PIPECONN *pc, *pi, *po, *pn;

//printf("console: (starting)\n");

    /* initialize this stage                                          */
    rc = xfl_stagestart(&pc);
    if (rc < 0) return 1;

    /* FIXME: ignoring command line arguments during development      */

    /* snag the first input stream and the first output stream        */
    pi = po = NULL;
    for (pn = pc; pn != NULL; pn = pn->next)
      { if (pn->flag & XFL_OUTPUT) { if (po == NULL) po = pn; }
        if (pn->flag & XFL_INPUT)  { if (pi == NULL) pi = pn; } }

    /* if pi is null then we are a first stage so slurp stdin         */
    if (pi == NULL && po == NULL)
    /* FIXME: provide an error message "no streams connected" */
//1493    E Too few streams are defined; &1 are present, but &2 are required
      { char *msgv[3];
        msgv[1] = "0"; msgv[2] = "1";
        xfl_error(1493,3,msgv,"CON");      /* provide specific report */
        return 1; }
    if (pi != NULL)
//printf("console: we seem to have an input stream\n");



/* streamstate RC == 12:   with no primary input we ARE a first stage */

    while (1) {

//printf("console: (top of loop)\n");
        buflen = sizeof(buffer) - 1;

        rc = xfl_peekto(pi,buffer,buflen);             /* sip on input */
        if (rc < 0) break; /* else */ buflen = rc;
        buffer[buflen] = 0x00;      /* terminate the string for stdout */
//printf("console: '%s' %d (input peeked)\n",buffer,rc);
printf(">>>%s<<<\n",buffer);

//      printf("%s\n",buffer);
//      if (po != NULL)
//      rc = xfl_output(po,buffer,buflen);       /* send it downstream */
//      if (rc < 0) break;

        xfl_readto(pi,NULL,0);     /* consume the record after sending */
//printf("console: (record consumed)\n");
      }
    if (rc < 0) return 1;

    /* terminate this stage cleanly                                   */
    rc = xfl_stagequit(pc);
    if (rc < 0) return 1;

//printf("console: (normal exit)\n");

    return 0;
  }



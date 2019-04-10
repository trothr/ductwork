/*
 *
 *        Name: console.c (C program source)
 *        Date: 2010-Mar-24
 *              Ductwork CONSOLE stage
 *
 */

#include <ductwork.h>

int s_console_fd0();
int s_console_fd1();
struct PIPECONN PI0, PO0;  /* side, ctrl, data, buff, glob, recno */

/* ------------------------------------------------------------------ */
int main()
  {
    static char _eyecatcher[] = "pipeline stage 'console' main()";

    /* initialize */
    (void) pipeline_stageinit();

    /* firstly, are we a first stage? */
    switch (pipeline_streamstate(&PI0))
      {
        case 12:  /* with no primary input, we ARE a first stage */
        return s_console_fd0();

        default:  /* otherwise, we are an end or intermediate */
        return s_console_fd1();
      }

  }

/* ---------------------------------------------------------------------
 *  We are READING from the "console" (that is, from FD0).
 */
int s_console_fd0()
  {
    static char _eyecatcher[] = "pipeline stage 'console' s_console_fd0()";

    char  recdat[65536];
    int  reclen;
    int  rc;

    /* "Do Forever" until we break out otherwise */
    while (1) {
        if (pipeline_streamstate(&PO0) == 12) break;
        rc = gets(recdat);
        rc = pipeline_output(&PO0,recdat,rc);
        /* break for EOF on input or output or for error */
      }

    /* pass a return code to our caller */
    return 0;
  }

/* ---------------------------------------------------------------------
 *  We are WRITING to the "console" (that is, to FD1).
 */
int s_console_fd1()
  {
    static char _eyecatcher[] = "pipeline stage 'console' s_console_fd1()";

    char  recdat[65536];
    int  reclen;
    int  rc;

    /* "Do Forever" until we break out otherwise */
    while (1) {
        /* perform a PEEKTO and see if there is a record ready */
        reclen = pipeline_peekto(&PI0,recdat,0);

        /* return value is size of the record or an error */
        if (reclen < 0) break;

        /* write the record to the "console" */
        puts(recdat);

        /* write the record to the output stream */
        rc = pipeline_output(&PO0,recdat,reclen);

        /* now consume the record from the input stream ala READTO */
        (void) pipeline_readto(&PI0,0,0);
      }

    /* pass a return code to our caller */
    return 0;
  }



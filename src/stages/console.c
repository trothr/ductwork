/*
 *
 *        Name: console.c (C program source)
 *        Date: 2010-Mar-24
 *              2021-Oct-18 (Mon)
 *              Ductwork CONSOLE stage
 *              This "stage" program serves as a 'console' interface
 *              for a Ductwork pipeline. Use it at the head of a pipeline
 *              to feed the pipeline from POSIX stdin. Use it at the
 *              tail of a pipeline to feed POSIX stdout from the pipe.
 *
 */

#include <ductwork.h>

int s_console_fd0();
int s_console_fd1();
struct PIPECONN PI0, PO0;  /* side, ctrl, data, buff, glob, recno */

/* ------------------------------------------------------------------ */
int main()
  {
    static char _eyecatcher[] = "Ductwork stage 'console' main()";

    /* initialize */
    (void) stageinit();

    /* firstly, are we a first stage? */
    switch (streamstate(&PI0))
      {
        case 12:  /* with no primary input, we ARE a first stage */
        return s_console_fd0();

        default:  /* otherwise, we are an end or intermediate */
        return s_console_fd1();
      }

  }

/* ---------------------------------------------------------------------
 *  We are READING from the "console" (that is, from FD0, stdin).
 */
int s_console_fd0()
  {
    static char _eyecatcher[] = "Ductwork stage 'console' s_console_fd0()";

    char  recdat[65536];
    int  reclen;
    int  rc;

    /* "Do Forever" until we break out otherwise */
    while (1) {
        if (streamstate(&PO0) == 12) break;
        rc = gets(recdat);
        rc = output(&PO0,recdat,rc);
        /* break for EOF on input or output or for error */
      }

    /* pass a return code to our caller */
    return 0;
  }

/* ---------------------------------------------------------------------
 *  We are WRITING to the "console" (that is, to FD1, stdout).
 */
int s_console_fd1()
  {
    static char _eyecatcher[] = "Ductwork stage 'console' s_console_fd1()";

    char  recdat[65536];
    int  reclen;
    int  rc;

    /* "Do Forever" until we break out otherwise */
    while (1) {
        /* perform a PEEKTO and see if there is a record ready */
        reclen = peekto(&PI0,recdat,0);

        /* return value is size of the record or an error */
        if (reclen < 0) break;

        /* write the record to the "console" */
        puts(recdat);

        /* write the record to the output stream */
        rc = output(&PO0,recdat,reclen);

        /* now consume the record from the input stream ala READTO */
        (void) readto(&PI0,0,0);
      }

    /* pass a return code to our caller */
    return 0;
  }



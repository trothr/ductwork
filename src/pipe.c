/*
 *
 *        (C program source)
 *        Date: week of VM Workshop
 *
 *
 *
 * The logic is as follows:
 * - process Unix-style options as individual argv elements
 * - concatenate remaining argv elements into a single string
 * - process CMS-style options which apply to the whole pipeline
 * - parse-out individual pipelines (if endchar is set)
 * - parse-out individual stages
 * - run all stages and wait for completion
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include <xpllib.h>

/* for development */
#include "xpllib.c"

/* -- this struct is only used in this program for the time being --- */
typedef struct PLINE {
    int num;                               /* number of this pipeline */
    char name[16];                           /* name of this pipeline */
    void *stage;    /* pointer to first stage struct in this pipeline */
    void *prev;                /* pointer to previous struct in chain */
    void *next;                /* pointer to next struct in the chain */
                     } PLINE;

/* -- this struct is only used in this program for the time being --- */
typedef struct STAGE {
    int num;              /* number of this stage within its pipeline */
    char name[16];                              /* name of this stage */
    void *pline;     /* pointer to struct of pipeline with this stage */
    void *prev;                /* pointer to previous struct in chain */
    void *next;                /* pointer to next struct in the chain */
    int argc;            /* count of arguments supplied to this stage */
//  char *argv[];        /* array of arguments supplied to this stage */
//  char **argv;         /* array of arguments supplied to this stage */
    char *argv[4];       /* array of arguments supplied to this stage */
          /* this array is intentionally limited to "verb" and "args" */
    struct PIPECONN *pc;            /* first in a chain of connectors */
                     } STAGE;

/* one pipeline with one stage:
Ready;
pipe foofum
FPLSCB027E Entry point FOOFUM not found
FPLSCA003I ... Issued from stage 1 of pipeline 1
FPLSCA001I ... Running "foofum"
Ready(-0027);
 */


/* ---------------------------------------------------------------------
 *  Similar calling convention to that of POSIX pipe() function.
 */
int xpl_pipe(PIPECONN**pc)
  {
    int rc, pdf[2], pdr[2];
    struct PIPECONN p0, *pi, *po;

    rc = pipe(pdf);                  /* create the forward POSIX pipe */
    if (rc < 0)                                 return -1;
    rc = pipe(pdr);                  /* create the reverse POSIX pipe */
    if (rc < 0) { close(pdf[0]); close(pdf[1]); return -1; }

    pi = malloc(sizeof(p0));          /* allocate the input connector */
    if (pi == NULL)                                /* check for error */
      { char *msgv[2], em[16]; int en;
        en = errno;
        perror("xpl_pipe(): malloc()");    /* provide standard report */
        /* also throw a pipelines/ductwork/plenum error and bail out  */
        sprintf(em,"%d",en); msgv[1] = em;       /* integer to string */
        xplerror(26,2,msgv,"LIB");         /* provide specific report */
        return -1; }

    po = malloc(sizeof(p0));         /* allocate the output connector */
    if (po == NULL)                                /* check for error */
      { char *msgv[2], em[16]; int en;
        en = errno;
        perror("xpl_pipe(): malloc()");    /* provide standard report */
        free(pi);      /* remember to free the other which had worked */
        /* also throw a pipelines/ductwork/plenum error and bail out  */
        sprintf(em,"%d",en); msgv[1] = em;       /* integer to string */
        xplerror(26,2,msgv,"LIB");         /* provide specific report */
        return -1; }

    pc[0] = pi;               /* "refers to the read end of the pipe" */
    pi->flag = XPL_INPUT;
    pi->fdf = pdf[0];          // reads content forward from producer */
    pi->fdr = pdr[1];          // writes control backward to producer */
    pi->prev = NULL;           /* pointer to previous struct in chain */
    pi->next = NULL;             /* this might be the head of a chain */

    pc[1] = po;              /* "refers to the write end of the pipe" */
    po->flag = XPL_OUTPUT;
    po->fdf = pdf[1];           // writes content forward to consumer */
    po->fdr = pdr[0];         // reads control backward from consumer */
    po->prev = NULL;           /* pointer to previous struct in chain */
    po->next = NULL;             /* this might be the head of a chain */

    return 0;
  }


/* ------------------------------------------------------------------ */
int main(int argc,char*argv[])
  {
    int rc, i;
    char *arg0, sep, end, bigbuf[65536], *p, *q;
    struct PLINE *pline, plin0, *plinx;
    struct STAGE *stage, stag0, *xtage;
    int plines, stages;
    struct PIPECONN *pc[2];

    sep = '|';                /* default stage separator is the usual */
    end = 0x00;                   /* default end character is not set */


    arg0 = argv[0];

    /* process Unix-style options as individual argv elements         */
    while (argc > 1 && *argv[1] == '-')
      {
        printf("option %s\n",argv[1]);

        if (strcmp(argv[1],"--end") == 0) { argc--; argv++; end = argv[1][0]; }

        argc--; argv++;
      }

printf("----- end '%c' sep '%c' -----\n",end,sep);

    /* allocate a struct for this pipeline */
    pline = malloc(sizeof(plin0));
    pline->prev = NULL;        /* pointer to previous struct in chain */
    pline->next = NULL;    /* this is (so far) last pipeline in chain */
    pline->stage = stage = NULL;    /* we will define the stages soon */
    pline->num = plines = 1;

    /* turn all arguments into one big string */
    p = bigbuf;
    for (i = 1; i < argc; i++)
      {
        if (i > 1) *p++ =  ' ';
//      printf("'%s'\n",argv[i]);
        q = argv[i];
        while (*q != 0x00) *p++ = *q++;
      }
    printf("'%s'\n",bigbuf);
printf("-----\n");

    /* HERE is where we parse CMS-style options */

    p = bigbuf;
    /* be sure we're at the first non-blank */
    while (*p == ' ') p++;

    stages = 0;
    while (1)
      {

        /* if first stage in this pipeline then start the chain       */
        if (stage == NULL) pline->stage = stage = malloc(sizeof(stag0));
        else          /* in any case allocate a struct for this stage */
          {
            stage->next = malloc(sizeof(stag0));
            stage = stage->next;
          }
        stage->prev = NULL;    /* pointer to previous struct in chain */
        stage->next = NULL;       /* this is (so far) last stage in chain */
        stage->num = ++stages;

        /* indicate the pipeline where this stage runs */
        stage->pline = pline;

    /* There are only two character sets we really care about here:   *
     * ASCII and EBCDIC. In both of those, blank and null collate     *
     * lower than all printable characters. So we can safely replace  *
     * blank with null when parsing this string.                      */

    q = p;
    while (*p != 0x00 && *p != sep) p++;
    if (*p == sep)
      {
        *p++ = 0x00;
        while (*p == ' ') p++;     /* skip to first non-blank in next */
        stage->next = malloc(sizeof(stag0));      /* allocate another */
        /* FIXME: check the return code */
      }

    /* the "verb" is the first non-blank token in the stage string    */
    stage->argv[0] = q; stage->argc = 1;
    while (*q != 0x00 && *q != ' ') q++;      /* skip to end of token */
    while (*q != 0x00 && *q == ' ') *q++ = 0x00;   /* null-out blanks */

    /* if there are arguments for this stage then pass them from here */
    if (*q != ' ' && *q != 0x00) {
stage->argv[1] = q; stage->argc = 2;
printf("'%s' '%s'\n",stage->argv[0],stage->argv[1]);
 } else
printf("'%s'\n",stage->argv[0]);

    if (stage->next == NULL) break;

      }

printf("-----\n");

    stage = pline->stage;
    while (stage != NULL)
      {
printf("freeing stage number %d\n",stage->num);
 xtage = stage->next; free(stage); stage = xtage; }

    while (pline != NULL)
      {
    printf("freeing pipeline number %d\n",pline->num);
 plinx = pline->next;
    free(pline);
    pline = plinx;
      }

printf("-----\n");

/* -- literal real pipeline | console ------------------------------- */

printf("\n");
printf("pipe literal real pipeline | console\n");
    rc = xpl_pipe(pc);

//>>>>>

    int sargc;
    char *sargv[16];

//  sargv[0] = "scaff2.sh";
//  sargc = 1;

    sargv[0] = "literal";
    sargv[1] = "real pipeline";
    sargc = 2;
    xplstagespawn(sargc,sargv,pc[1],pc[0]);   /* first stage writes to #1 */

    sargv[0] = "console";
    sargv[1] = "";
    sargc = 2;
    xplstagespawn(sargc,sargv,pc[0],pc[1]);   /* second stage reads from #0 */

//<<<<<

    close(pc[0]->fdf);
    close(pc[0]->fdr);
    free(pc[0]);

    close(pc[1]->fdf);
    close(pc[1]->fdr);
    free(pc[1]);


printf("\n");


//printf("dun?\n");

    return 0;
  }



/*
 *
 *        Name: pipe.c (C program source)
 *              This is called the "launcher" because "dispatcher" is
 *              not the right term. On POSIX systems, we rely on the
 *              operating system kernel to do the actual dispatching.
 *        Date: week of the VM Workshop and I don't remember which year
 *
 * The logic is as follows:
 * - process Unix-style options as individual argv elements
 * - concatenate remaining argv elements into a single string
 * - process CMS-style options which apply to the whole pipeline
 * - parse-out individual stages
 * - parse-out individual pipelines (if endchar is set)
 * - run all stages and wait for completion
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <xfl.h>

int main(int argc,char*argv[])
  {
    int rc, i, nullokay;
    char *arg0, *args, *p;
    char *escape, *endchar, *stagesep, *pipename;   /* separator */
    char *msgv[2];

    nullokay = 0;            /* null pipeline is *not* initially okay */

    /* inherit defaults established by parent or by the user */
    escape = getenv("PIPEOPT_ESCAPE");             /* default is none */
    if (escape == NULL)                                     escape = "";
    endchar = getenv("PIPEOPT_ENDCHAR");           /* default is none */
    if (endchar == NULL)                                   endchar = "";
    stagesep = getenv("PIPEOPT_SEPARATOR");         /* default is bar */
    if (stagesep == NULL || *stagesep == 0x00)           stagesep = "|";

    /* remeober argv[0] for use later */
    arg0 = argv[0];
//printf("argv[0]='%s'\n",arg0);
//printf("argc=%d\n",argc);

    while (argc > 1 && *argv[1] == '-')
      {
        if (strcmp(argv[1],"--version") == 0)              /* VERSION */
          { int xfl_version = XFL_VERSION;
            int vv, rr, mm;
            char verstr[16];
            vv = (xfl_version >> 24) & 0xff;   /* extract the version */
            rr = (xfl_version >> 16) & 0xff;   /* extract the release */
            mm = (xfl_version >> 8) & 0xff;  /* extract the mod level */
            sprintf(verstr,"%d.%d.%d",vv,rr,mm);
            msgv[1] = verstr;
            xfl_error(86,2,msgv,"PIP");    /* provide specific report */
            nullokay = 1; } else
        if (strcmp(argv[1],"--escape") == 0)                /* ESCAPE */
          { if (argc < 3) { printf("error\n"); return 1; }
            escape = argv[2]; argc--; argv++; } else
        if (strcmp(argv[1],"--endchar") == 0)              /* ENDCHAR */
          { if (argc < 3) { printf("error\n"); return 1; }
            endchar = argv[2]; argc--; argv++; } else
        if (strcmp(argv[1],"--stagesep") == 0)            /* STAGESEP */
          { if (argc < 3) { printf("error\n"); return 1; }
            stagesep = argv[2]; argc--; argv++; } else
        if (strcmp(argv[1],"--separator") == 0)          /* SEPARATOR */
          { if (argc < 3) { printf("error\n"); return 1; }
            stagesep = argv[2]; argc--; argv++; } else
        if (strcmp(argv[1],"--name") == 0)                    /* NAME */
          { if (argc < 3) { printf("error\n"); return 1; }
            pipename = argv[2]; argc--; argv++; } else

          { /* 0014 E Option &1 not valid */
//printf("BOGUS OPTION %s\n",argv[1]);
            msgv[1] = argv[1];
            xfl_error(14,2,msgv,"PIP"); /* 0014 E Option &1 not valid */
            return 1; }

        argc--; argv++;
      }

    /* string-up all arguments into one */
printf("argc=%d\n",argc);
    args = xfl_argcat(argc,argv);             /* must eventually free */
    if (args == NULL) /* error, then */ return 1;

    p = args;
    while (*p != ' ' && *p != 0x00) p++;

    /* if we have CMS-style options then process them here and now    */
    if (*p == '(')
      {
        p++;
        while (1)
          {
            while ((*p == ' ' || *p == '\t') && *p != 0x00) p++;

            if (strncmp(p,"ESCAPE",3) == 0)                 /* ESCAPE */
              { while (*p != ' ' || *p == '\t' && *p != 0x00) p++;
                while ((*p == ' ' || *p == '\t') && *p != 0x00) p++;
                escape = p; } else
            if (strncmp(p,"ENDCHAR",3) == 0)               /* ENDCHAR */
              { while (*p != ' ' || *p == '\t' && *p != 0x00) p++;
                while ((*p == ' ' || *p == '\t') && *p != 0x00) p++;
                endchar = p; } else
            if (strncmp(p,"STAGESEP",2) == 0)             /* STAGESEP */
              { while (*p != ' ' || *p == '\t' && *p != 0x00) p++;
                while ((*p == ' ' || *p == '\t') && *p != 0x00) p++;
                stagesep = p; } else
            if (strncmp(p,"NAME",1) == 0)                     /* NAME */
              { while (*p != ' ' || *p == '\t' && *p != 0x00) p++;
                while ((*p == ' ' || *p == '\t') && *p != 0x00) p++;
                stagesep = p; } else

              { /* 0014 E Option &1 not valid */
//printf("BOGUS OPTION %s\n",argv[1]);
                msgv[1] = argv[1];
                xfl_error(14,2,msgv,"PIP");    /* Option &1 not valid */
                return 1; }

            argc--; argv++;
          }
      }

printf("escape='%s'\n",escape);   /* FIXME: --escape/ESCAPE should be xorc */
printf("endchar='%s'\n",endchar);   /* FIXME: --endchar/ENDCHAR should be xorc */
printf("separator='%s'\n",stagesep);   /* FIXME: --stagesep/STAGESEP should be xorc */
printf("argc=%d\n",argc);

printf("args='%s'\n",args);
    if (*p == 0x00)   /* if empty string then we have a null pipeline */
      {
        if (nullokay) return 0;
        xfl_error(12,2,msgv,"PIP");           /* 0012 E Null pipeline */
        return 1; 
      }

/*
int xfl_trace(int,int,char**,char*);
int xfl_pipepair(PIPECONN*[]);
int xfl_getpipepart(PIPESTAGE**,char*);
int xfl_stagespawn(int,char*[],PIPECONN*[]);
 */

    free(args);

    return 0;
  }

/*
label logic
                  A:   -- labels a stream
                  A: | -- connects labeled stage to an input
                | A:   -- connects labeled stage to an output
                | A: | -- is illegal
 */

/* one pipeline with one stage:
Ready;
pipe foofum
            msgv[1] = "FOOFUM";
            xfl_error(27,2,msgv,"PIP"); 0027 E Entry point &1 not found
FPLSCB027E Entry point FOOFUM not found
            msgv[1] = "1";
            msgv[2] = "1";
            xfl_error(3,3,msgv,"PIP"); 0003 I ... Issued from stage &1 of pipeline &2
FPLSCA003I ... Issued from stage 1 of pipeline 1
            msgv[1] = "foofum";
            xfl_error(1,2,msgv,"PIP"); 0001 I ... Running "&1"
FPLSCA001I ... Running "foofum"
Ready(-0027);
 */

#ifdef OLDSTUFF

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

/* ------------------------------------------------------------------ */
int main(int argc,char*argv[])
  {
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




printf("\n");

    return 0;
  }

#endif


/*
0059 E Logical record length &1 is not valid
0183 E Output buffer overflow; &1 required
0093 E Pipeline not installed as a nucleus extension; use PIPE command (BASH extension)
0050 E Not a character or hexadecimal representation: &1
 */

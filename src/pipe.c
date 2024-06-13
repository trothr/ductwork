/*
 *
 *        Name: pipe.c (C program source)
 *        Date: 2023, 2024, and prior, and following
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
#include <unistd.h>
#include <sys/wait.h>

#include <xfl.h>

extern struct PIPECONN *xfl_pipeconn;
extern struct PIPESTAGE *xfl_pipestage;

int main(int argc,char*argv[])
  {
    int rc, i, nullokay;
    char *arg0, *args, *p, *q, *r;
    char *escape, *endchar, *stagesep, *pipename, *dotrace;
    char *msgv[2];
    struct PIPECONN *pi, *px;
    int wpid, wstatus;

    nullokay = 0;            /* null pipeline is *not* initially okay */

    /* inherit defaults established by parent or by the user */
    escape = getenv("PIPEOPT_ESCAPE");             /* default is none */
    if (escape == NULL)                                     escape = "";
    endchar = getenv("PIPEOPT_ENDCHAR");           /* default is none */
    if (endchar == NULL)                                   endchar = "";
    stagesep = getenv("PIPEOPT_SEPARATOR");         /* default is bar */
    if (stagesep == NULL || *stagesep == 0x00)           stagesep = "|";

    pipename = dotrace = "";

    /* remeober argv[0] for use later */
    arg0 = argv[0];

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

        if (strcmp(argv[1],"--trace") == 0)                  /* TRACE */
            dotrace = "YES"; else

          { /* 0014 E Option &1 not valid */
            msgv[1] = argv[1];
            xfl_error(14,2,msgv,"PIP"); /* 0014 E Option &1 not valid */
            return 1; }

        argc--; argv++;
      }

    /* string-up all arguments into a single string which we ...      */
    args = xfl_argcat(argc,argv);         /* ... must eventually free */
    if (args == NULL) { perror("xfl_argcat()"); return 1; }

    /* skip to first non-blank in the full arguments string           */
    p = args;
    while ((*p == ' ' && *p == '\t') && *p != 0x00) p++;
    r = p;

    /* if we have CMS-style options then process them here and now    */
    if (*p == '(')
      {
        /* skip to end of CMS-style options, closing parenthesis      */
        while (*r != 0x00 && *r != ')') r++;
        if (*r != 0x00) *r++ = 0x00;      /* terminate options string */

        p++;
        while (*p != 0x00)
          {
            /* skip to next non-blank character in CMS style args     */
            while ((*p == ' ' || *p == '\t') && *p != 0x00) p++;
            q = p;                        /* hold onto start of token */

            /* find end of this blank-delimited token                 */
            while (*p != ' ' && *p != '\t' && *p != 0x00) p++;
            if (*p != 0x00) *p++ = 0x00;         /* mark end of token */

            if (strncasecmp(q,"ESCAPE",3) == 0)             /* ESCAPE */
              { /* skip to next non-blank character in CMS style args */
                while ((*p == ' ' || *p == '\t') && *p != 0x00) p++;
                if (*p != 0x00) escape = p++;    /* here is the value */
                /* find the end of the blank-delimited value          */
                while (*p != ' ' && *p != '\t' && *p != 0x00) p++;
                if (*p != 0x00) *p++ = 0x00; } else

            if (strncasecmp(q,"ENDCHAR",3) == 0)           /* ENDCHAR */
              { while ((*p == ' ' || *p == '\t') && *p != 0x00) p++;
                if (*p != 0x00) endchar = p++;
                while (*p != ' ' && *p != '\t' && *p != 0x00) p++;
                if (*p != 0x00) *p++ = 0x00; } else

            if (strncasecmp(q,"STAGESEP",2) == 0)         /* STAGESEP */
              { while ((*p == ' ' || *p == '\t') && *p != 0x00) p++;
                if (*p != 0x00) stagesep = p++;
                while (*p != ' ' && *p != '\t' && *p != 0x00) p++;
                if (*p != 0x00) *p++ = 0x00; } else

            if (strncasecmp(q,"NAME",1) == 0)                 /* NAME */
              { while ((*p == ' ' || *p == '\t') && *p != 0x00) p++;
                if (*p != 0x00) pipename = p++;
                while (*p != ' ' && *p != '\t' && *p != 0x00) p++;
                if (*p != 0x00) *p++ = 0x00; } else

              { /* 0014 E Option &1 not valid */
                msgv[1] = q;
                xfl_error(14,2,msgv,"PIP");    /* Option &1 not valid */
                return 1; }
          }
      }
    /* skip to first non-blank character after all options            */
    while ((*r == ' ' || *r == '\t') && *r != 0x00) r++;

printf("escape='%s'\n",escape);   /* FIXME: --escape/ESCAPE should be xorc */
printf("endchar='%s'\n",endchar);   /* FIXME: --endchar/ENDCHAR should be xorc */
printf("separator='%s'\n",stagesep);   /* FIXME: --stagesep/STAGESEP should be xorc */

printf("arg0='%s'\n",arg0);
printf("argc=%d\n",argc);
printf("args='%s'\n",r);
    if (*p == 0x00)   /* if empty string then we have a null pipeline */
      { if (nullokay) return 0;
        xfl_error(12,2,msgv,"PIP");           /* 0012 E Null pipeline */
        return 1; }


    /* if tracing was requested then set this environment variable    */
    if (*dotrace != 0x00) setenv("PIPEOPT_TRACE",dotrace,1);



/*
int xfl_trace(int,int,char**,char*);
int xfl_pipepair(PIPECONN*[]);
int xfl_getpipepart(PIPESTAGE**,char*);
int xfl_stagespawn(int,char*[],PIPECONN*[]);
 */

    /* shut it all down */

    free(args);

    /* remember to free the connectors too ... at least close FDs     */
    px = xfl_pipeconn;
    i = 0 ; while (px != NULL)
      {
        i = i + 1;
        close(px->fdf);
        close(px->fdr);
        pi = px;
        px = px->next;
        free(pi);
      }

    /* wait for stages to complete */

    while (1)
      {
        rc = wpid = waitpid(-1,&wstatus,0);
        if (rc < 1) break;
        printf("pipe: stage with PID %d finished\n",wpid);
      }
    if (rc < 0) perror("waitpid()");

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







/*****************************************************************************/






#ifdef _TO_MERGE

/*
extern struct PIPECONN *xfl_pipeconn;
extern struct PIPESTAGE *xfl_pipestage;
 */

/* ------------------------------------------------------------------ */
int main(int argc,char*argv[])
  {
    int rc, snum, pnum, pend, i;
    char *args, *p, *q, *msgv[16];
    struct PIPECONN *pp[3], *pi, *po, *px;
    struct PIPESTAGE *sx;
    int wpid, wstatus;



//printf("plenum: the pipeline >>>\n%s\n<<<\n",args);
    msgv[1] = args;
    xfl_trace(3000,2,msgv,"PIP");

    /* start with some defaults */
    stagesep = '|'; streamsep = '!'; snum = pnum = 1;
    p = q = args;
    pp[0] = pp[1] = pp[2] = px = NULL;    /* start with no connectors */


    /* step through the pipeline specification string                 */
    while (*p != 0x00)
      {
        while (*p != 0x00 && *p != stagesep && *p != streamsep) p++;
        /* we have a stage ... might be the only one ... or a stream end */

        pi = pp[0];         /* input here is output of previous stage */
                                 /* it's the read side of the PC pair */
if (pi != NULL && px == NULL) px = pi;      /* not sure this is right */

pend = 0;

        if (*p == stagesep)   /* we have a follow-on stage, get ready */
          {
            *p++ = 0x00;                 /* terminate this sub-string */
            xfl_pipepair(pp);        /* get a new connector pair pp[] */
            po = pp[1];     /* our output is next's input (write end) */
          } else {            /* we are the last stage in this stream */
            if (*p && (*p == streamsep))
              { /* the following three need to be done AFTER stage stacking */
                pend = 1;
                pnum = pnum + 1;            /* bump the stream number */
//              snum = 1;                   /* reset the stage number */
//              pp[0] = pp[1] = NULL;   /* start next w no connectors */
              }
            /* in any case ... */
            if (*p) *p++ = 0x00;         /* terminate this sub-string */
            po = NULL;       /* and we have no follow-on so no output */
                 }

        /* stack this stage */
          {
            int arqc, i;
            char *arqv[3], *r, *l;
            struct PIPESTAGE ps0, *ps; /* ps = &ps0; */

            r = q;
            /* skip past any leading white space */
            while (*r == ' ' || *r == '\t')               r++;

            /* peel-off any stage label */
            l = r;
            while (*r != ' ' && *r != '\t' && *r != ':' && *r != 0x00) r++;
            if (*r == ':')
              {
                *r++ = 0x00;         /* delimit label and advance pointer */
            /* skip past any leading white space */
            while (*r == ' ' || *r == '\t')               r++;
              } else {
                r = l;
                l = "";
                     }

            arqv[0] = r;         /* stage verb */
            while (*r != ' ' && *r != '\t' && *r != 0x00) r++;
            if (*r != 0x00) *r++ = 0x00;
            arqv[1] = r;         /* stage args */
            if (*r == 0x00) arqc = 1; else arqc = 2;
//printf("verb '%s' args '%s'\n",arqv[0],arqv[1]);
//printf("plenum: %s: %s\n",l,arqv[0]);

        /* get a new struct for this stage */
        xfl_getpipepart(&ps,l);
if (ps == NULL) printf("error\n");

              {
                char *v0, *v1;
                v0 = arqv[0]; v1 = ps->arg0;
                if (v0 == NULL) v0 = "";
                if (v1 == NULL) v1 = "";
if (*v0 && *v1) printf("plenum: ERROR: multiple commands on a stage\n");
              }
            if (arqv[0] != NULL && *arqv[0] != 0x00)
              { ps->arg0 = arqv[0];
                if (arqv[1] != NULL && *arqv[1] != 0x00)
                    ps->args = arqv[1]; }

//printf("plenum: PC counters %d %d\n",ps->ipcc,ps->opcc);
            if (pi != NULL)
              { ps->ipcv[ps->ipcc] = pi;
                ps->ipcc = ps->ipcc + 1;
                ps->ipcv[ps->ipcc] = NULL;       /* mark end of chain */
                ps->xpcv[ps->xpcc] = pi;
                ps->xpcc = ps->xpcc + 1;
                ps->xpcv[ps->xpcc] = NULL; }     /* mark end of chain */
            if (po != NULL)
              { ps->opcv[ps->opcc] = po;
                ps->opcc = ps->opcc + 1;
                ps->opcv[ps->opcc] = NULL;       /* mark end of chain */
                ps->xpcv[ps->xpcc] = po;
                ps->xpcc = ps->xpcc + 1;
                ps->xpcv[ps->xpcc] = NULL; }     /* mark end of chain */
//printf("plenum: PC counters %d %d\n",ps->ipcc,ps->opcc);
//printf("   pi = %08X;    po = %08X; %s\n",pi,po,ps->arg0);
          }


        stagetot++;                    /* bump stagenum for reporting */
        if (pend)       /* if end of stream then prep for next stream */
          {
                snum = 1;                   /* reset the stage number */
                pp[0] = pp[1] = NULL;   /* start next w no connectors */
          }
        else    snum = snum + 1;      /* bump stagenum for next cycle */
        q = p;    /* set q to point to next, if any */
      }
//printf("\n");           /* development */
//printf("plenum: total stages %d (%d final)\n",stagetot,snum);
//printf("plenum: total streams %d\n",pnum);

/* -- END OF PARSING ------------------------------------------------ */

    /* be sure that stages won't get whacked by SIGPIPE on connectors */
    signal(SIGCHLD,SIG_IGN);

    /* launch all stacked/queued stages */
    i = 0; sx = xfl_pipestage;
    while (sx != NULL)
      {
        int c; char *a;
        char *arqv[3];
        void *v;

        a = sx->args;
        if (a != NULL && *a != 0x00) c = 2; else c = 1;
        arqv[0] = sx->arg0;
        arqv[1] = sx->args;
        arqv[2] = NULL;

//      pi = sx->ipcv[0];   /* pi */
//      po = sx->opcv[0];   /* po */

//      a = sx->label; if (a == NULL) a = "";
//printf("plenum: %s: %s\n",a,sx->arg0);
//printf("plenum: %s %s\n",sx->arg0,sx->args);

        v = sx->xpcv;
        xfl_stagespawn(c,arqv,v);

        i = i + 1;
        sx = sx->next;
      }
//printf("plenum: %d stages launched by parent\n",i);

//  if (args != NULL) free(args);           /* see xfl_argcat() above */



sleep(5);

    return 0;
  }

#endif



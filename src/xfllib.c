/*
 *
 *        Name: xfllib.c (C program source)
 *              Ductwork library
 *        Date: 2023-06-12 (Monday)
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <fcntl.h>

#include "configure.h"

#include <xmitmsgx.h>
struct MSGSTRUCT xflmsgs;
char *xmmprefix = PREFIX;

#include "xfl.h"

/* ---------------------------------------------------------------------
 *  This is a byte-at-a-time read function which attempts to consume
 *  a single line of text and no more.
 */
ssize_t readl(int fd, void *buf, size_t count)
  {
    static char _eyecatcher[] = "readl()";
    int i, rc;
    char *p;

    i = 0; p = buf;
    while (i < count)
      {
        /* try to read just one byte from the socket */
        rc = read(fd,p,1);
        /* this is a tight spin while the stack gives us empty reads  */
        while (rc == 0) rc = read(fd,p,1);
        /* if we got an error then return now and indicate the fact   */
        if (rc < 0) return rc;
        /* treat a newline as end-of-string */
        if (*p == '\n') *p = 0x00;
        /* upon finding end-of-string then return */
        if (*p == 0x00) break;
        /* ignore carriage return but for all others bump the pointer */
        if (*p == '\r') { i++; p++; }
      }
    return i;
  }

/* ---------------------------------------------------------------------
 *  NOTE: this routine allocates a string buffer
 *        which the caller must eventually free to avoid memory leaks.
 *  NOTE: this routine ignores argv[0] so that you can call it
 *        directly from main() with supplied argc/argv as-is.
 */
char*xfl_argcat(int argc,char*argv[])
  {
    static char _eyecatcher[] = "catargs()";
    int i, buflen;
    char *buffer, *p, *q;

    /* account for intervening blanks between tokens                  */
    buflen = argc;
    /* result is plus one for all args, allowing for the terminator,  */
    /* but also one more for argv[0] which is harmless, but we later  */
    /* have to remember to back-off the count by TWO for real length  */

    /* add-up the sizes of the tokens                                 */
    for (i = 1; i < argc; i++) buflen = buflen + strlen(argv[i]);

    /* allocate a buffer to hold the combined tokens as one string    */
    buffer = malloc(buflen);

    if (buffer == NULL)
      { char *msgv[2], em[16]; int en;
        en = errno;    /* hold onto the error value in case it resets */
        perror("catargs(): malloc()");     /* provide standard report */
        /* also throw a pipelines/ductwork/plenum error and bail out  */
        sprintf(em,"%d",en); msgv[1] = em;       /* integer to string */
        xfl_error(26,2,msgv,"LIB");        /* provide specific report */
        return NULL; }

    /* insert the first token into the string                         */
    p = buffer;
    if (argc > 1) { q = argv[1]; while (*q) *p++ = *q++; }

    /* now concatenate all remaining tokens delimited with blanks     */
    for (i = 2; i < argc; i++)
      { *p++ = ' ';        /* insert a blank space between the tokens */
                    q = argv[i]; while (*q) *p++ = *q++; }
    *p = 0x00;

    return buffer;
  }

#ifdef BYPASS
int xfl_argcat(int argc,char*argv[],char**args)
  {
    int i, j;

    /* first scan argv for lengths of all arguments */
    j = 0;
    for (i = 1; i < argc; i++) { j = j + strlen(argv[i]) + 1; }
    /* then concatenate the arguments into one single string */
    *args = NULL;
    return 0;
  }
#endif

/* ---------------------------------------------------------------------
 */
int xfl_error(int msgn,int msgc,char*msgv[],char*caller)
  {
    static char _eyecatcher[] = "xfl_error()";
    char msgbuf[256];
    int rc;

    rc = xmopen("xfl",0,&xflmsgs);

    /* some functions indicate the error with a negative number       */
    if (msgn < 0) msgn = 0 - msgn;

    /* populate the message struct - some of this is outside the API  */
    xflmsgs.msgnum = msgn;
    xflmsgs.msgc = msgc;
    xflmsgs.msgv = (unsigned char**) msgv;
    xflmsgs.msgbuf = msgbuf;
    xflmsgs.msglen = sizeof(msgbuf) - 1;

    /* do we need these? */
    xflmsgs.msglevel = 0;

    /* using pfxmaj and pfxmin is definitely outside the XMITMSGX API */
    strncpy(xflmsgs.pfxmaj,"XFL",4);
    strncpy(xflmsgs.pfxmin,caller,4);
    /* also remember to up-case that */

    /* make the message */
    rc = xmmake(&xflmsgs);
    if (rc != 0) return rc;

    /* print it */
    fprintf(stderr,"%s\n",msgbuf);

    return 0;
  }

/* ----------------------------------------------------------- STAGEEXEC
 *   Called by: ...
 *       Calls: ...
 *
 * NOTE: this routine is destructive to the argument string supplied
 */
int xfl_stageexec(char*args,PIPECONN*pc[])
  {
    int rc;
    char *verb, *argv[3], pipeconn[256], pipeprog[256], *p;
    PIPECONN *pi, *po;

    /* skip past any leading white space */
    while (*args == ' ' || *args == '\t') args++;
    verb = args;
    while (*args != 0x00 && *args != ' ' && *args != '\t') args++;
    if (*args != 0x00) *args++ = 0x00;

//printf("xfl_stageexec(): verb '%s'\n",verb);
if (args != NULL && *args != 0x00)
//printf("xfl_stageexec(): args '%s'\n",args);
    argv[0] = verb;
    argv[1] = args;
    argv[2] = NULL;

    p = pipeconn;
    pi = pc[0];
    po = pc[1];
    if (pi != NULL)
      {
//      printf("*.INPUT:%d,%d\n",pi->fdf,pi->fdr);
        rc = sprintf(p,"*.INPUT:%d,%d ",pi->fdf,pi->fdr);
//      if (rc < 0) ...
        p = &p[rc];
      }
    if (po != NULL)
      {
//      printf("*.OUTPUT:%d,%d\n",po->fdf,po->fdr);
        rc = sprintf(p,"*.OUTPUT:%d,%d ",po->fdf,po->fdr);
//      if (rc < 0) ...
        p = &p[rc];
      }
    setenv("PIPECONN",pipeconn,1);
//system("sh -c set | grep PIPECONN");

    sprintf(pipeprog,"stages/%s",verb);
    execv(pipeprog,argv);
/*
    execve(pipeprog,argv,NULL);
 */




    return 0;
  }

/* ---------------------------------------------------------- STAGESPAWN
 *   Called by: ...
 *       Calls: the stage indicated in argv[0]
 */
int xfl_stagespawn(int argc,char*argv[],PIPECONN*pi[],PIPECONN*po[],PIPECONN*pk[])
  /* argc - count of arguments much like Unix/POSIX main()            */
  /* argv - argument array much like Unix/POSIX main()                */
  /* pi   - pipe connector(s) this stage will use for input           */
  /* po   - pipe connector(s) this stage will use for output          */
  /* pk   - pipe connector(s) to kill-off before spawhing this stage  */
  {
    int rc, i;
    char *p, *q, envbuf[8192], tmpbuf[256], pipepath[8192];

    rc = fork();
    if (rc < 0) return errno;       /* negative return code: an error */
    if (rc > 0) return 0;       /* positive return code: PID of child */
    /* FIXME: we should probably file that child PID for use later    */
    /* and finally, return code zero means we are the child process   */

//  myname = NULL;
    p = envbuf;

    /* process input connectors */
    i = 0; while (pi[i] != NULL)
      { if (pi[i]->flag & XFL_OUTPUT)
{ printf("fail\n"); exit(1); }
        sprintf(tmpbuf,"*.INPUT.%d:%d,%d",i,pi[i]->fdf,pi[i]->fdr);

        /* copy this token into the environment variable buffer       */
        q = tmpbuf;
        while (*q != 0x00) *p++ = *q++; *p++ = ' ';                    }

    /* process output connectors */
    i = 0; while (po[i] != NULL)
      { if (po[i]->flag & XFL_INPUT)
{ printf("fail\n"); exit(1); }
        sprintf(tmpbuf,"*.OUTPUT.%d:%d,%d",i,po[i]->fdf,po[i]->fdr);

        /* copy this token into the environment variable buffer       */
        q = tmpbuf;
        while (*q != 0x00) *p++ = *q++; *p++ = ' ';                    }

    /* prepare to pass connector info to the stage */
    *p = 0x00;                                /* terminate the string */
    setenv("PIPECONN",envbuf,1);

    /* scan PIPEPATH for the stage ($PREFIX/lib/stages) */
    strncpy(pipepath,getenv("PIPEPATH"),sizeof(pipepath)-1);



    if (argc > 1) q = argv[1];
             else q = "";
    sprintf(tmpbuf,"stages/%s %s&",argv[0],q);
    system(tmpbuf);

    return 0;
  }

/* ---------------------------------------------------------- STAGESTART
 * initialize the internal input and output connectors (two fd each)
 */
int xfl_stagestart(PIPECONN**pc)
  {
    char *p, *pipeconn, number[16];
    struct PIPECONN pc0, *pc1, *pcp;
    int i;

    *pc = NULL;
    pcp = NULL;

    pipeconn = getenv("PIPECONN");
    /* for the time being it is okay having connections established   */
    if (pipeconn == NULL) return 0;
//printf("stagestart: PIPECONN='%s'\n",pipeconn);

    /* parse the connections passed to this stage in the environment  */
    p = pipeconn;
    while (*p != 0x00 && *p != ' ')
      {
        if (*p == '*') p++;        /* skip past "*." to I/O indicator */
        if (*p == '.') p++;            /* else throw error number 191 */

        if (*p == 'I' || *p == 'i') pc0.flag = XFL_INPUT;
        if (*p == 'O' || *p == 'o') pc0.flag = XFL_OUTPUT;
//0100    E Direction "&1" not input or output

        while (*p != 0x00 && *p != ' ' && *p != '.' && *p != ':') p++;
//      pc0.name[0] = 0x00; pc0.n = 0;  // see notes, development
//      if (*p == '.')
//        { p++;
//          for (i = 0; i < sizeof(pc0.name) - 1 &&
//                      *p != 0x00 && *p != ' ' && *p != '.' && *p != ':' && *p != ','; i++)
//              pc0.name[i] = *p++;
//          pc0.name[i] = 0x00; }
//printf("pc0.name = '%s'\n",pc0.name);
        /* if there is a name and it is numeric then it is a number   */
        if (*p == ':')                 /* else throw error number 193 */
          { p++;
            number[0] = 0x00;
            for (i = 0; i < sizeof(number) - 1 &&
                        *p != 0x00 && *p != ' ' && *p != '.' && *p != ':' && *p != ','; i++)
                number[i] = *p++;
            number[i] = 0x00; pc0.fdf = atoi(number); }
        if (*p == ',')
          { p++;
            number[0] = 0x00;
            for (i = 0; i < sizeof(number) - 1 &&
                        *p != 0x00 && *p != ' ' && *p != '.' && *p != ':' && *p != ','; i++)
                number[i] = *p++;
            number[i] = 0x00; pc0.fdr = atoi(number); }

        pc0.next = NULL;
        pc0.prev = *pc;

        /* allocate the connector struct for hand-off                 */
        pc1 = malloc(sizeof(pc0));
        // check for error
        if (*pc == NULL) *pc = pc1;
              else pcp->next = pc1;
        memcpy(pc1,&pc0,sizeof(pc0));
        pcp = pc1;

        while (*p != 0x00 && *p == ' ') p++;
      }

    return 0;
  }

/* ------------------------------------------------------------ PIPEPAIR
 * Conceptually similar to POSIX pipe() function, returns two ends.
 */
int xfl_pipepair(PIPECONN*pp[])
  {
    struct PIPECONN p0, *pi, *po;
    int fdf[2], fdr[2];

    /* we need *two* traditional POSIX/Unix pipes */
    pipe(fdf);                  /* forward for data */
    pipe(fdr);                  /* reverse for control */
    /* FIXME: need to check for errors after these calls */

    /* establish the side used for input */
    pi = malloc(sizeof(p0));    /* pipeline input */
    if (pi == NULL)
      { char *msgv[2], em[16]; int en;
        en = errno;    /* hold onto the error value in case it resets */
        perror("xfl_pipepair(): malloc()");        /* standard report */
        sprintf(em,"%d",en); msgv[1] = em;       /* integer to string */
        xfl_error(26,2,msgv,"LIB");        /* provide specific report */
        return en; }
    pi->fdf /* read  */ = fdf[0]; /* data forward */
    pi->fdr /* write */ = fdr[1]; /* control back */
//printf("xfl_pipepair(): *.INPUT:%d,%d\n",pi->fdf,pi->fdr);
    pi->flag = XFL_INPUT;

    /* establish the side used for output */
    po = malloc(sizeof(p0));    /* pipeline output */
    if (po == NULL)
      { char *msgv[2], em[16]; int en;
        en = errno;    /* hold onto the error value in case it resets */
        perror("xfl_pipepair(): malloc()");        /* standard report */
        free(pi);     /* the other malloc() worked so free that block */
        sprintf(em,"%d",en); msgv[1] = em;       /* integer to string */
        xfl_error(26,2,msgv,"LIB");        /* provide specific report */
        return en; }
    po->fdf /* write */ = fdf[1]; /* data forward */
    po->fdr /* read  */ = fdr[0]; /* control back */
//printf("xfl_pipepair(): *.OUTPUT:%d,%d\n",po->fdf,po->fdr);
    po->flag = XFL_OUTPUT;

    /* follow POSIX pipe() semantics: two plenum connectors           */
    pp[0] = pi;                 /* [0] refers to the read end */
    pp[1] = po;                 /* [1] refers to the write end */
    /* see 'man 2 pipe' on most Unix or Linux systems for the idea    */

    return 0;
  }

/* ----------------------------------------------------------- STAGEQUIT
 *  do an orderly close of the file descriptors and release of storage
 */
int xfl_stagequit(PIPECONN*pc)
  {
    struct PIPECONN *pn;

//printf("xfl_stagequit: hello\n");

    while (pc != NULL)
      {
        /* if an input connector then signal the producer to quit     */
        if (pc->flag && XFL_INPUT)
          { printf("xfl_stagequit: signaling consumer to sever the connection\n");
             write(pc->fdr,"QUIT",sizeof("QUIT")); }
        else printf("xfl_stagequit: output side, no signal\n");

        /* in any case close the forward and reverse file descriptors */
        printf("xfl_stagequit: closing file descriptors\n");
        close(pc->fdf); close(pc->fdr);

        /* proceed to next struct in the chain and free this one      */
        pn = pc->next;
        free(pc);
        pc = pn;
      }

//printf("xfl_stagequit: goodbye\n");

    return 0;
  }

/* -------------------------------------------------------------- PEEKTO
 *  CONSUMER SIDE
 *  Returns: number of bytes in the record or negative for error
 *  A return value of zero is not an error if the record was null.
 *  See also: xfl_readto() and xfl_output()
 */
int xfl_peekto(PIPECONN*pc,void*buffer,int buflen)
  { static char _eyecatcher[] = "xfl_peekto()";
    int  rc, reclen;
    char  infobuff[256];

    /* be sure we are on the input side of the connection             */
    if ((pc->flag && XFL_INPUT) == 0)
      { printf("xfl_peekto: called for a non-input connector\n");
        return -1; }  //FIXME: get a better return code
//printf("peekto: okay but bailing out for development\n");
//return -614;

/*

"STAT" ** the only meta data at this point in the development
          producer sends number of bytes available

"PEEK" ** think PIPLOCAT to examine a record
          producer sends data

"NEXT" ** think PIPINPUT (sort of) consume the record
          producer advances the sequence count

"QUIT" ** for SEVER operation
          producer closes file descriptors

"FAIL" ** if something went wrong

 */

    /* PROTOCOL:                                                      */
    /* direct the producer to report the size of this record */
//  rc = write(pc->fdr,"STAT",sizeof("STAT"));
    rc = write(pc->fdr,"STAT",4);
    if (rc < 0)
      { char *msgv[2], em[16];
        rc = errno; if (rc == 0) rc = -1;
        perror("peekto(): write():");      /* provide standard report */
        /* also throw a pipelines/ductwork/plenum error and bail out  */
        sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
        xfl_error(26,2,msgv,"LIB");         /* provide specific report */
        return rc; }
//printf("xfl_peekto: sent STAT control %d\n",rc);

    /* PROTOCOL:                                                      */
    /* read the response which should simply have an int string */
    rc = read(pc->fdf,infobuff,sizeof(infobuff));
    if (rc < 0)
      { char *msgv[2], em[16];
        rc = errno; if (rc == 0) rc = -1;
        perror("peekto(): read():");       /* provide standard report */
        /* also throw a pipelines/ductwork/plenum error and bail out  */
        sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
        xfl_error(26,2,msgv,"LIB");         /* provide specific report */
        return rc; }
    infobuff[rc] = 0x00;
printf("xfl_peekto: infobuff = '%s'\n",infobuff);

    /* convert integer string into a binary integer */
//  if (*infobuff is non-digit) then set this connector to close

    if (isdigit(*infobuff))
    reclen = atoi(infobuff);
//  else { /* shutdown */ }

printf("xfl_peekto: expecting %d bytes\n",reclen);

    /* undocumented feature: zero-length peekto tells the record size */
    if (buflen == 0) return reclen;

    /* does the supplied buffer have room for this record? */
    if (buflen < reclen) return -1;

    /* PROTOCOL:                                                      */
    /* direct the producer to send the record content */
//  rc = write(pc->fdr,"PEEK",sizeof("PEEK"));
    rc = write(pc->fdr,"PEEK",4);
    if (rc < 0)
      { char *msgv[2], em[16];
        rc = errno; if (rc == 0) rc = -1;
        perror("peekto(): write():");      /* provide standard report */
        /* also throw a pipelines/ductwork/plenum error and bail out  */
        sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
        xfl_error(26,2,msgv,"LIB");         /* provide specific report */
        return rc; }
printf("xfl_peekto: sent PEEK; expecting %d bytes\n",reclen);

    /* PROTOCOL:                                                      */
    rc = read(pc->fdf,buffer,reclen);
    if (rc < 0)
      { char *msgv[2], em[16];
        rc = errno; if (rc == 0) rc = -1;
        perror("peekto(): read():");       /* provide standard report */
        /* also throw a pipelines/ductwork/plenum error and bail out  */
        sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
        xfl_error(26,2,msgv,"LIB");         /* provide specific report */
        return rc; }

    return rc;
  }

/* -------------------------------------------------------------- READTO
 *  CONSUMER SIDE
 *  Returns: number of bytes in the record or negative for error
 *  A return value of zero is not an error if the record was null.
 *  See also: xfl_peekto() and xfl_output()
 */
int xfl_readto(PIPECONN*pc,void*buffer,int buflen)
  { static char _eyecatcher[] = "xfl_readto()";
    int  rc;
    char  infobuff[256];

    /* be sure we are on the input side of the connection             */
    if ((pc->flag && XFL_INPUT) == 0)
      { printf("xfl_readto: called for a non-input connector\n");
        return -1; } // FIXME: get a better return code

    /* if buffer supplied and length not zero then try to get data    */
    if (buffer != NULL && buflen > 0)
      { rc = xfl_peekto(pc,buffer,buflen);
        if (rc < 0) return rc; }
    /* this also checks things like which side this connector is for  */

    /* PROTOCOL:                                                      */
    /* direct the producer to proceed with the next record */
//  rc = write(pc->fdr,"NEXT",sizeof("NEXT"));
    rc = write(pc->fdr,"NEXT",4);
    if (rc < 0)
      { char *msgv[2], em[16];
        rc = errno; if (rc == 0) rc = -1;
        perror("readto(): write():");      /* provide standard report */
        /* also throw a pipelines/ductwork/plenum error and bail out  */
        sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
        xfl_error(26,2,msgv,"LIB");         /* provide specific report */
        return rc; }

    /* PROTOCOL:                                                      */
    /* read the response which should be simply "OKAY" */
//  rc = read(pc->fdf,infobuff,sizeof(infobuff));
//  if (rc < 0)
//    { char *msgv[2], em[16];
//      rc = errno; if (rc == 0) rc = -1;
//      perror("readto(): read():");       /* provide standard report */
//      /* also throw a pipelines/ductwork/plenum error and bail out  */
//      sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
//      xfl_error(26,2,msgv,"LIB");         /* provide specific report */
//      return rc; }

    /* increment the record counter */
    pc->rn = pc->rn + 1;

    return 0;
  }

/* -------------------------------------------------------------- OUTPUT
 *  PRODUCER SIDE
 *  Returns: number of bytes written to output or negative for error
 *  A return value of zero is not an error if the record was null.
 *  This routine sits in a loop driven by the consumer.
 *  See also: xfl_peekto() and xfl_readto()
 */
int xfl_output(PIPECONN*pc,void*buffer,int buflen)
  { static char _eyecatcher[] = "xfl_output()";
    int rc, xx;
    char  infobuff[256];
int n;

    /* be sure we are on the output side of the connection            */
    if ((pc->flag && XFL_OUTPUT) == 0)
//    { xfl_error(61,0,NULL,"LIB");     /* provide specific report */
      { printf("xfl_output: called for a non-output connector\n");
        return -1; } // FIXME: get a better return code

printf("xfl_output: '%s' %d %d\n",buffer,buflen,strlen(buffer));

n = 0;
    while (1)
      {
n = n + 1;
        /* the following is a blocking read; this routine waits until *
         * the consumer side signals that it is ready to consume      */
//      rc = read(pc->fdr,infobuff,sizeof(infobuff));
        rc = 0; while (rc == 0)
        rc = read(pc->fdr,infobuff,4);    /* expect 4 bytes by design */
        if (rc < 4)
          { char *msgv[2], em[16];
//          rc = errno; if (rc == 0) rc = -1;
//          perror("output(): read():");       /* provide standard report */
            /* also throw a pipelines/ductwork/plenum error and bail out  */
            sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
            xfl_error(26,2,msgv,"LIB");         /* provide specific report */
printf("xfl_output: error trying to read the control channel after %d %d\n",n,rc);
            return rc; }
        infobuff[rc] = 0x00;
printf("xfl_output: infobuff = '%s'\n",infobuff);

        xx = 0;
        switch (*infobuff)
          {
            case 'S': case 's':                               /* STAT */
                /* PROTOCOL: send the size of the record              */
                sprintf(infobuff,"%d",buflen);   /* say # bytes avail */
                rc = write(pc->fdf,infobuff,strlen(infobuff));
                break;

            case 'P': case 'p':                               /* PEEK */
                /* PROTOCOL: send the record downstream               */
                rc = write(pc->fdf,buffer,buflen);   /* send the data */
                break;

            case 'N': case 'n':                               /* NEXT */
                /* PROTOCOL: acknowledge to consumer we unblocked     */
//              rc = write(pc->fdf,"OKAY",4);
// need to flag "done" here
                rc = 0;
                xx = 1;
                break;

            case 'Q': case 'q':                               /* QUIT */
printf("xfl_output: got a quit signal from the consumer\n");
                /* PROTOCOL: consumer has signaled a sever            */
//              write(pc->fdf,"OKAY",4);
                close(pc->fdf);
                close(pc->fdr);
                rc = 0;
                xx = 1;
                break;

            default:
printf("xfl_output: protocol error '%s'\n",infobuff);
// need to indicate a protocol error here
                rc = -1;
                xx = 1;
                break;
          }

        if (rc < 0)
          { char *msgv[2], em[16];
            rc = errno; if (rc == 0) rc = -1;
            perror("xfl_output(): write():");         /* system report */
            /* also throw a pipelines/ductwork/plenum error and bail  */
            sprintf(em,"%d",rc); msgv[1] = em;   /* integer to string */
            xfl_error(26,2,msgv,"LIB");     /* provide specific report */
printf("xfl_output: error after protocol\n");
            return rc; }

        if (xx) break;
      }

    /* increment the record counter */
    pc->rn = pc->rn + 1;

//printf("xfl_output: (normal exit)\n");

    return 0;
  }

/* ---------------------------------------------------------------------
NOT SURE THIS IS ACTUALLY NEEDED
 */
int xfl_parse(char*args,int*optc,char*optv[],char*rest)
  {
    return 0;
  }


/* -------------------------------------------------------------- ABBREV
 * Returns length of info if info is an abbreviation of informat.
 * Returns zero if info does not match or is shorter than minlen.
 * Compare to abbrev() this version being *not* case sensitive.
 */
int xfl_abbrevi(char*informat,char*info,int minlen)
  {
    static char _eyecatcher[] = "abbrevi()";

    int     i;
    for (i = 0; info[i] != 0x00; i++)
       if (tolower((int)informat[i]) != tolower((int)info[i])) return 0;
    if (i < minlen) return 0;
    return i;
  }



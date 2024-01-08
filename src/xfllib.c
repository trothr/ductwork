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

#include <xmitmsgx.h>
struct MSGSTRUCT xflmsgs;

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
        en = errno;
        perror("catargs(): malloc()");     /* provide standard report */
        /* also throw a pipelines/ductwork/plenum error and bail out  */
        sprintf(em,"%d",en); msgv[1] = em;       /* integer to string */
        xfl_error(26,2,msgv,"LIB");         /* provide specific report */
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

xmopen("xfl",0,&xflmsgs);

    /* some functions indicate the error with a negative number       */
    if (msgn < 0) msgn = 0 - msgn;

    /* populate the message struct - some could be outside of the API */
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

    rc = xmmake(&xflmsgs);
printf("xmmake() returned %d\n",rc);
    if (rc != 0) return rc;

    fprintf(stderr,"%s\n",msgbuf);

    return 0;
  }

/* ---------------------------------------------------------- STAGESPAWN
 */
int xplstagespawn(int argc,char*argv[],PIPECONN*pc,PIPECONN*pk)
  /* argc - count of arguments much like Unix/POSIX main()            */
  /* argv - argument array much like Unix/POSIX main()                */
  /* pc   - pipe connector(s) to this stage will use                  */
  /* pk   - pipe connector(s) to kill-off before spawhing this stage  */
  {
    char *myside, *myname, *p, *q, envbuf[8192], tmpbuf[256];

    myname = NULL;
    p = envbuf;
    while (pc != NULL)
      {
        /* set "INPUT" or "OUTPUT" tag based on side element          */
//      if (pc->side) myside = "OUTPUT"; else myside = "INPUT";
        if (pc->flag & XFL_OUTPUT) myside = "OUTPUT";
        if (pc->flag & XFL_INPUT)  myside = "INPUT";
        /* if the connector is named then put the name into the token */
        if (myname != NULL && myname != 0x00)
        sprintf(tmpbuf,"*.%s.%s:%d,%d",myside,myname,pc->fdf,pc->fdr);
        else
        sprintf(tmpbuf,"*.%s:%d,%d",myside,pc->fdf,pc->fdr);
        /* copy this token into the environment variable buffer       */
        q = tmpbuf;
        while (*q != 0x00) *p++ = *q++; *p++ = ' ';
        /* now bump to the next PC structure in the chain             */
        pc = pc->next;
      }
    *p = 0x00;                                /* terminate the string */

    setenv("PIPECONN",envbuf,1);

    if (argc > 1) q = argv[1];
             else q = "";
    sprintf(tmpbuf,"stages/%s %s&",argv[0],q);
    system(tmpbuf);

    return 0;
  }

/* ---------------------------------------------------------- STAGESTART
 */
int xplstagestart(PIPECONN**pc)
  {
    char *p, *pipeconn, number[16];
    struct PIPECONN pc0, *pc1;
    int i;

    *pc = NULL;

    pipeconn = getenv("PIPECONN");
    /* for the time being it is okay having connections established   */
    if (pipeconn == NULL) return 0;
//printf("PIPECONN='%s'\n",pipeconn);

    /* parse the connections passed to this stage in the environment  */
    p = pipeconn;
    while (*p != 0x00 && *p != ' ')
      {
//    PIPECONN='*.INPUT:3,6 '
        if (*p == '*') p++;
        if (*p == '.') p++;            /* else throw error number 191 */
        if (*p == 'I' || *p == 'i') pc0.flag = XFL_INPUT;
        if (*p == 'O' || *p == 'o') pc0.flag = XFL_OUTPUT;
        while (*p != 0x00 && *p != ' ' && *p != '.' && *p != ':') p++;
        pc0.name[0] = 0x00; pc0.n = 0;
        if (*p == '.')
          { p++;
            for (i = 0; i < sizeof(pc0.name) - 1 &&
                        *p != 0x00 && *p != ' ' && *p != '.' && *p != ':' && *p != ','; i++)
                pc0.name[i] = *p++;
            pc0.name[i] = 0x00; }
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

//printf("fdf = %d\n",pc0.fdf);
//printf("fdr = %d\n",pc0.fdr);

        pc0.next = NULL;
        pc0.prev = *pc;

        /* allocate the connector struct for hand-off                 */
        pc1 = malloc(sizeof(pc0));
        // check for error
        if (*pc == NULL) *pc = pc1;
        memcpy(pc1,&pc0,sizeof(pc0));







        while (*p != 0x00 && *p == ' ') p++;
      }



    return 0;
  }

/* ----------------------------------------------------------- STAGEQUIT
 *  do an orderly close of the file descriptors and release of storage
 */
int xplstagequit(PIPECONN*pc)
  {
    struct PIPECONN *pn;

printf("xfl_stagequit: hello\n");

    while (pc != NULL)
      {
        /* if an input connector then signal the producer to quit     */
        if (pc->flag && XFL_INPUT) write(pc->fdr,"QUIT",sizeof("QUIT"));

        /* in any case close the forward and reverse file descriptors */
        close(pc->fdf); close(pc->fdr);

        /* proceed to next struct in the chain and free this one      */
        pn = pc->next;
        free(pc);
        pc = pn;
      }

printf("xfl_stagequit: goodbye\n");

    return 0;
  }

/* -------------------------------------------------------------- PEEKTO
 *  CONSUMER SIDE
 *  Returns: number of bytes in the record or negative for error
 */
int xfl_peekto(PIPECONN*pc,void*buffer,int buflen)
  {
    static char _eyecatcher[] = "xfl_peekto()";
    int  rc, reclen;
    char  infobuff[256];

    /* be sure we are on the input side of the connection             */
    if ((pc->flag && XFL_INPUT) == 0) return -1;    //FIXME: get a better return code
//printf("readto: okay but bailing out for development\n");
//return -614;

/*

"STAT" ** the only meta data at this point in the development
          producer sends "DATA seq bytes"

"PEEK" ** think PIPLOCAT to examine a record
          producer sends data

"NEXT" ** think PIPINPUT (sort of) consume the record
          producer sends "OKAY" and advances the sequence count

"QUIT" ** for SEVER operation
          producer sends "OKAY"

"FAIL" ** if something went wrong

          consumer may also receive a "FAIL" from any of the above

 */


    /* PROTOCOL:                                                      */
    /* direct the producer to report the size of this record */
    rc = write(pc->fdr,"STAT",sizeof("STAT"));
//  rc = write(pc->fdr,"STAT",strlen("STAT")+1);
    if (rc < 0)
      { char *msgv[2], em[16];
        rc = errno; if (rc = 0) rc = -1;
        perror("peekto(): write():");      /* provide standard report */
        /* also throw a pipelines/ductwork/plenum error and bail out  */
        sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
        xfl_error(26,2,msgv,"LIB");         /* provide specific report */
        return rc; }
printf("xfl_readto: sent STAT control %d\n",rc);

    /* PROTOCOL:                                                      */
    /* read the response which should simply have an int string */
    rc = read(pc->fdf,infobuff,sizeof(infobuff));
    if (rc < 0)
      { char *msgv[2], em[16];
        rc = errno; if (rc = 0) rc = -1;
        perror("peekto(): read():");       /* provide standard report */
        /* also throw a pipelines/ductwork/plenum error and bail out  */
        sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
        xfl_error(26,2,msgv,"LIB");         /* provide specific report */
        return rc; }
    infobuff[rc] = 0x00;
printf("xfl_readto: infobuff = '%s'\n",infobuff);

    /* convert integer string into a binary integer */
//  if (*infobuff is non-digit) then set this connector to close

    if (isdigit(*infobuff))
    reclen = atoi(infobuff);
//  else { /* shutdown */ }


printf("xfl_readto: expecting %d bytes\n",reclen);

    /* undocumented feature: zero-length peekto tells the record size */
    if (buflen == 0) return reclen;

    /* does the supplied buffer have room for this record? */
    if (buflen < reclen) return -1;

    /* PROTOCOL:                                                      */
    /* direct the producer to send the record content */
    rc = write(pc->fdr,"PEEK",sizeof("PEEK"));
//  rc = write(pc->fdr,"PEEK",strlen("PEEK")+1);
    if (rc < 0)
      { char *msgv[2], em[16];
        rc = errno; if (rc = 0) rc = -1;
        perror("peekto(): write():");      /* provide standard report */
        /* also throw a pipelines/ductwork/plenum error and bail out  */
        sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
        xfl_error(26,2,msgv,"LIB");         /* provide specific report */
        return rc; }
printf("xfl_readto: sent PEEK control\n");


printf("xfl_readto: expecting %d bytes\n",reclen);
    /* PROTOCOL:                                                      */
    rc = read(pc->fdf,buffer,reclen);
    if (rc < 0)
      { char *msgv[2], em[16];
        rc = errno; if (rc = 0) rc = -1;
        perror("peekto(): read():");       /* provide standard report */
        /* also throw a pipelines/ductwork/plenum error and bail out  */
        sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
        xfl_error(26,2,msgv,"LIB");         /* provide specific report */
        return rc; }
//  buffer[reclen] = 0x00;


    return rc;
  }


/* -------------------------------------------------------------- READTO
 *  CONSUMER SIDE
 *  Returns: number of bytes in the record or negative for error
 */
int xfl_readto(PIPECONN*pc,void*buffer,int buflen)
  {
    static char _eyecatcher[] = "xfl_readto()";
    int  rc;
    char  infobuff[256];


    if (buffer != NULL && buflen > 0)
      {
        rc = xfl_peekto(pc,buffer,buflen);
        if (rc < 0) return rc;
      }

    /* PROTOCOL:                                                      */
    /* direct the producer to proceed with the next record */
    rc = write(pc->fdr,"NEXT",sizeof("NEXT"));
//  rc = write(pc->fdr,"NEXT",strlen("NEXT")+1);
    if (rc < 0)
      { char *msgv[2], em[16];
        rc = errno; if (rc = 0) rc = -1;
        perror("peekto(): write():");      /* provide standard report */
        /* also throw a pipelines/ductwork/plenum error and bail out  */
        sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
        xfl_error(26,2,msgv,"LIB");         /* provide specific report */
        return rc; }


    /* PROTOCOL:                                                      */
    /* read the response which should be simply "OKAY" */
    rc = read(pc->fdf,infobuff,sizeof(infobuff));
    if (rc < 0)
      { char *msgv[2], em[16];
        rc = errno; if (rc = 0) rc = -1;
        perror("readto(): read():");       /* provide standard report */
        /* also throw a pipelines/ductwork/plenum error and bail out  */
        sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
        xfl_error(26,2,msgv,"LIB");         /* provide specific report */
        return rc; }

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
  {
    static char _eyecatcher[] = "xfl_output()";
    int rc, xx;
    char  infobuff[256];

    /* be sure we are on the output side of the connection            */
    if ((pc->flag && XFL_OUTPUT) == 0) return -1;    //FIXME: get a better return code
//return -615;

printf("xfl_output: '%s' %d %d\n",buffer,buflen,strlen(buffer));











    while (1)
      {

        rc = read(pc->fdr,infobuff,sizeof(infobuff));
        if (rc < 0)
          { char *msgv[2], em[16];
            rc = errno; if (rc = 0) rc = -1;
            perror("output(): read():");       /* provide standard report */
            /* also throw a pipelines/ductwork/plenum error and bail out  */
            sprintf(em,"%d",rc); msgv[1] = em;       /* integer to string */
            xfl_error(26,2,msgv,"LIB");         /* provide specific report */
            return rc; }
        infobuff[rc] = 0x00;
printf("xfl_output: infobuff = '%s'\n",infobuff);

        xx = 0;
        switch (*infobuff)
          {
            case 'S': case 's':                               /* STAT */
                /* PROTOCOL: send the size of the record              */
                sprintf(infobuff,"%d",buflen);
                rc = write(pc->fdf,infobuff,strlen(infobuff));
                break;

            case 'P': case 'p':                               /* PEEK */
                /* PROTOCOL: send the record downstream               */
                rc = write(pc->fdf,buffer,buflen);
                break;

            case 'N': case 'n':                               /* NEXT */
                /* PROTOCOL: acknowledge to consumer we unblocked     */
                rc = write(pc->fdf,"OKAY",sizeof("OKAY"));
//              rc = write(pc->fdf,"OKAY",strlen("OKAY")+1);
// need to flag "done" here
                xx = 1;
                break;

            case 'Q': case 'q':                               /* QUIT */
                /* PROTOCOL: consumer has signaled a sever            */
                write(pc->fdf,"OKAY",sizeof("OKAY"));
//              write(pc->fdf,"OKAY",strlen("OKAY")+1);
                close(pc->fdf);
                close(pc->fdr);
                rc = 0;
                xx = 1;
                break;

            default:
// need to indicate a protocol error here
                rc = -1;
                xx = 1;
                break;
          }

        if (rc < 0)
          { char *msgv[2], em[16];
            rc = errno; if (rc = 0) rc = -1;
            perror("xfl_peekto(): write():");         /* system report */
            /* also throw a pipelines/ductwork/plenum error and bail  */
            sprintf(em,"%d",rc); msgv[1] = em;   /* integer to string */
            xfl_error(26,2,msgv,"LIB");     /* provide specific report */
            return rc; }

        if (xx) break;
      }



    /* increment the record counter */
    pc->rn = pc->rn + 1;

printf("xfl_output: (normal exit)\n");

    return 0;
  }



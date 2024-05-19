/*
 *
 *        Name: xfl.h (C program header)
 *        Date: 2023-06-12 (Mon) roughly, 2024-01-07 (Sun)
 *
 *

From: Nancy Foley <nfoley@us.ibm.com>
To: Rick Troth <rmt@casita.net>
Subject: Prefix Request
Date: Wed, 26 Jul 2023 14:37:20 +0000

Prefix XFL* has been assigned to you and your product.
If/when you have a product number assigned, please let me know
so that I may update the prefix database accordingly. Thanks.

Nancy J. Foley
element@us.ibm.com
IBM C-S 0-2043-003
zStack Custom Build, Software Operations
IBM Systems

 */

#ifndef _XFLLIB_H

//static char *_ductwork_version = "Ductwork 0.7.6";
#define  XFL_VERSION  (((0) << 24) + ((7) << 16) + ((6) << 8) + (0))
//static int xfl_version = XFL_VERSION;

#define     XFL_F_INPUT         0x0001
#define     XFL_F_OUTPUT        0x0002
#define     XFL_F_KEEP          0x0010
#define     XFL_F_SEVERED       0x0020  /* possible EPIPE */

#define     XFL_E_NONE          0
#define     XFL_E_SEVERED       12

#define     XFL_MAX_STREAMS  16

typedef struct PIPECONN {

    int fdf;         /* forward flowing file descriptor used for data */
                 /* data flows "downstream" from producer to consumer */
    int fdr;      /* reverse flowing file descriptor used for control */
                 /* control goes "upstream" from consumer to producer */
    int flag;   /* which side of the connection, producer or consumer */

    char name[16];            /* name of connector for a named stream */
    int n;               /* number of connector for a numbered stream */

    int pline, pstep;          /* which pipeline and which step/stage */
    int cpid;             /* PID of child process which inherited FDs */

    int rn;   /* record number - how many records have been processed */
    /* This leads to 1-based indexing because when this is zero       */
    /* nothing has happened yet. So there is no "record zero".        */

    void *buff;        /* optional buffer for shared memory transfers */
    void *glob;                                        /* global area */
    void *prev;                /* pointer to previous struct in chain */
    void *next;                /* pointer to next struct in the chain */

                        } PIPECONN;

typedef struct PIPESTAGE {
    char *text;                       /* string describing this stage */
//  int plinenumb;                  /* pipeline where this stage runs */
//  int stagenumb;                /* number of this stage in its line */
    char *label;                          /* pointer to label, if any */
    char *arg0;                          /* executable name or "verb" */
    char *args;                                   /* arguments string */
//  int argc;
//  char **argv;
    int  ipcc;                          /* input pipe connector count */
    void *ipcv[16];              /* input pipe connector vector array */
    int  opcc;                         /* output pipe connector count */
    void *opcv[16];             /* output pipe connector vector array */
    int  xpcc;                         /* COMMON pipe connector count */
    void *xpcv[16];             /* COMMON pipe connector vector array */

    int cpid;             /* PID of child process handling this stage */

    void *prev;                /* pointer to previous struct in chain */
    void *next;                /* pointer to next struct in the chain */
                         } PIPESTAGE;

/* --- function prototypes ------------------------------------------ */

char*xfl_argcat(int,char*[]);                   /* FKA xplcatargs     */

int xfl_error(int,int,char**,char*);
int xfl_pipepair(PIPECONN*[]);          /* allocate an in/out pair */
int xfl_getpipepart(PIPESTAGE**,char*);

int xfl_stagespawn(int,char*[],PIPECONN*[]);

int xfl_stagestart(PIPECONN**);           /* returns a pipeconn array */
int xfl_peekto(PIPECONN*,void*,int);      /* pipeconn, buffer, buflen */
int xfl_readto(PIPECONN*,void*,int);      /* pipeconn, buffer, buflen */
int xfl_output(PIPECONN*,void*,int);      /* pipeconn, buffer, buflen */
int xfl_stagequit(PIPECONN*);          /* releases the pipeconn array */
int xfl_sever(PIPECONN*);                   /* disconnect a connector */


#define _XFLLIB_H
#endif


/*   discard    http://code.google.com/p/ductwork/                    */

/********************************************************************************/

int pipeline_streamstate(PIPECONN*);
int pipeline_sever(PIPECONN*);
int pipeline_addpipe();

/*
int     pipe(int *fd);
int     poll(void *pollfds, long nfds, int timeout);
 */

/*

consumer sends:
"STAT" ** the only meta data at this point in the development
          producer sends "DATA seq bytes", number of current record and how big

consumer sends:
"PEEK" ** think PIPLOCAT to examine a record
          producer sends data

consumer sends:
"NEXT" ** think PIPINPUT (sort of) consume the record
          producer sends data and advances the sequence count

consumer sends:
"QUIT" ** for SEVER operation
          producer closes file descriptors
          consumer closes file descriptors

consumer sends:
"FAIL" ** if something went wrong

          consumer may also receive a "FAIL" from any of the above
          as long as it is not misunderstood as data (like after STAT)

 */

/* error codes
 -2756  Too many operands.
 -2811  A stream with the stream identifier specified is already defined.
 */

/*
labels
                  A:   -- labels a stream
                  A: | -- connects labeled stage to an input
                | A:   -- connects labeled stage to an output
                | A: | -- is illegal
 */



/*
 *
 *        Name: xfl.h (C program header)
 *        Date: 2023-06-12 (Mon) roughly, 2024-01-07 (Sun)
 *
 *

**

From: Nancy Foley <nfoley@us.ibm.com>
To: "rmt@casita.net" <rmt@casita.net>
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

#define     XFL_INPUT   0x0001
#define     XFL_OUTPUT  0x0002

#define     XFL_MAX_STREAMS  16

typedef struct PIPECONN {

    int fdf;         /* forward flowing file descriptor used for data */
                 /* data flows "downstream" from producer to consumer */
    int fdr;      /* reverse flowing file descriptor used for control */
                 /* control goes "upstream" from consumer to producer */
    int flag;   /* which side of the connection, producer or consumer */

//  char name[16];            /* name of connector for a named stream */
//  int n;               /* number of connector for a numbered stream */

    int rn;   /* record number - how many records have been processed */
    /* This leads to 1-based indexing because when this is zero       */
    /* nothing has happened yet. So there is no "record zero".        */

    void *buff;        /* optional buffer for shared memory transfers */
    void *glob;                /* global area */
    void *prev;                /* pointer to previous struct in chain */
    void *next;                /* pointer to next struct in the chain */

                        } PIPECONN;


char*xfl_argcat(int,char*[]);                   /* FKA xplcatargs     */

int xfl_error(int,int,char**,char*);
int xfl_pipepair(PIPECONN*[]);          /* allocate an in/out pair */

int xfl_stagestart(PIPECONN**);
int xfl_peekto(PIPECONN*,void*,int);      /* pipeconn, buffer, buflen */
int xfl_readto(PIPECONN*,void*,int);      /* pipeconn, buffer, buflen */
int xfl_output(PIPECONN*,void*,int);      /* pipeconn, buffer, buflen */
int xfl_stagequit(PIPECONN*);

/* the following are still in development */
int xfl_stageexec(char*,PIPECONN*[]);
int xplstagespawn(int,char*[],PIPECONN*[],PIPECONN*[],PIPECONN*[]);

int xfl_parse(char*,int*,char*[],char*);

#define _XFLLIB_H
#endif



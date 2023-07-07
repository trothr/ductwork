/*
 *
 *        Name: xpllib.h (C program header)
 *        Date: 2023-06-12 (Mon) roughly
 *
 *
 */

#ifndef _XPLLIB_H

#define     XPL_INPUT   0x0001
#define     XPL_OUTPUT  0x0002

//static char *_ductwork_version = "Ductwork 0.7.5";
#define  XPL_VERSION  (((0) << 24) + ((7) << 16) + ((5) << 8) + (0))

typedef struct PIPECONN {

    int fdf;         /* forward flowing file descriptor used for data */
                 /* data flows "downstream" from producer to consumer */
    int fdr;      /* reverse flowing file descriptor used for control */
                 /* control goes "upstream" from consumer to producer */
    int flag;   /* which side of the connection, producer or consumer */

    char name[16];            /* name of connector for a named stream */
    int n;               /* number of connector for a numbered stream */

    int rn;   /* record number - how many records have been processed */
    /* This leads to 1-based indexing because when this is zero       */
    /* nothing has yet happened. So there is no "record zero".        */

    void *buff;        /* optional buffer for shared memory transfers */
    void *glob;                /* global area */
    void *prev;                /* pointer to previous struct in chain */
    void *next;                /* pointer to next struct in the chain */

                        } PIPECONN;

int xplerror(int,int,char**,char*);
int xploutput(PIPECONN*,void*,int);
int xplpeekto(PIPECONN*,void*,int);
int xplreadto(PIPECONN*,void*,int);

char*xplcatargs(int,char**);
int xplstagespawn(int,char**,PIPECONN*,PIPECONN*);
int xplstagestart(PIPECONN**);

int xplstagequit(PIPECONN*);

#define _XPLLIB_H
#endif



/*
 *
 *        Name: literal.c
 *              safely concatenates all arguments into a single string
 *              and then writes that string to the output
 *        Date: 2023-06-09 (Friday) Gallatin
 *
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* for development */
#include <stdio.h>

#include <ductwork.h>

int main(int argc,char*argv[])
  {
    int i, buflen;
    char *buffer, *p, *q;

    /* account for intervening blanks between tokens                  */
    buflen = argc;
    /* result is plus one for all args, allowing for the terminator,  */
    /* but also one more for argv[0] which is harmless, but we later  */
    /* have to remember to back-off the count by TWO when writing     */

    /* add-up the sizes of the tokens                                 */
    for (i = 1; i < argc; i++) buflen = buflen + strlen(argv[i]);

    /* allocate a buffer to hold the combined tokens as one string    */
    buffer = malloc(buflen);
    if (buffer == NULL)
      { perror("malloc()");       /* provide a standard system report */
        /* throw a pipelines/ductwork/plenum error and bail out       */
//      fpxstageerror(errno);
        return 1; }

    /* insert the first token into the string                         */
    p = buffer;
    if (argc >= 2)
      { q = argv[1]; while (*q) *p++ = *q++; *p = 0x00; }

    /* now concatenate all remaining tokens delimited with blanks     */
    for (i = 2; i < argc; i++)
      { *p++ = ' ';        /* insert a blank space between the tokens */
        q = argv[i]; while (*q) *p++ = *q++; *p = 0x00; }

    /* back-off buffer length for to write the correct record length  */
    buflen = buflen - 2;

printf(">%s< %d %d\n",buffer,buflen,strlen(buffer));

    /* initialize this stage, though we only need primary output      */
//  rc = fpxstageinit();

    /* write this record that we have labored so hard to create       */
//  rc = fpxstageoutput(pc,buffer,buflen);

    free(buffer);

    return 0;
  }



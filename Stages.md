# Ductwork Stages

This document lists a number of Ductwork stages
which must be included in a basic implementation.

## Ductwork Stages

* console

The `console` stage is so named for compatibility with CMS/TSO Pipelines.
In Ductwork, 'console' serves as a gateway between shell pipes and Ductwork stages.

When `console` is the first stage of a pipeline, it reads lines of text from file descriptor zero (0)
delimited by newline characters. In this mode, its output must be connected
to another Ductwork stage and its output is records with newline characters removed.
Output records are bounded where newlines occur on input.

When `console` is NOT the first stage of a pipeline,
its input must be connected to another Ductwork stage. It then reads input records
and writes to file descriptor one (1) with newline characters appended to each line.
In this mode, 'console' may also be connected on output to a following Ductwork stage
to which it will write the input records unaltered. (No newline inserted.)


* hole

Use the 'hole' stage to safely consume all records form preceeding stages.



* buffer

Use the `buffer` stage to hold all input records
until the source stage terminates.






* block

* deblock

* fblock


* drop

* take



* fanout

* fanin

* faninany







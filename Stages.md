# XFL Stages

This document lists a number of POSIX Pipelines stages
which must be included in a basic implementation.

## XFL Stages

The currently available stages are listed here in no particular order.

* console

The `console` stage is so named for compatibility with CMS/TSO Pipelines.
In POSIX Pipelines, `console` serves as a gateway between shell pipes and POSIX Pipelines stages.

When `console` is the first stage of a pipeline, it reads lines of text from file descriptor zero (0)
delimited by newline characters. In this mode, its output must be connected
to another POSIX Pipelines stage and its output is records with newline characters removed.
Output records are bounded where newlines occur on input.

When `console` is NOT the first stage of a pipeline,
its input must be connected to another POSIX Pipelines stage. It then reads input records
and writes to file descriptor one (1) with newline characters appended to each line.
In this mode, `console` may also be connected on output to a following POSIX Pipelines stage
to which it will write the input records unaltered. (No newline inserted.)

* hole

Use the `hole` stage to safely consume all records form preceeding stages.

* buffer

Use the `buffer` stage to hold all input records
until the source stage terminates.

* append

Use the `append` stage to copy all input records to its output
and then convey the output of the appended stage.

* block

Use the `block` stage to encapsulate records in any of several
pre-defined formats.

* deblock

Use the `deblock` stage to reverse the effect of `block`.

* fblock

Use the `fblock` stage to reformat the bytes in a stream into
fixed-length records. Input records will be split or joined as needed
to form output records of identical size. The final record can be
optionally padded.

* drop

Use the `drop` stage to discard records from the stream.
You can drop any number of records from the begining or from the end.

`drop` is inverse to `take`.

* take

Use the `take` stage to selectively pass records in the stream.
You can take any number of records at the begining or end of the stream.

`take` is inverse to `drop`

* fanout

Use the `fanout` stage to duplicate the input stream
into multiple outputs. `fanout` reads from its primary input
and copies each record to all connected output streams.

* fanin

Use the `fanin` stage to collect multiple input streams
into a single output.

* faninany

Use the `faninany` stage to collect from multiple input streas
as each is available. `faninany` reads from whichever input stream
has data available and writes that record to its output stream.

Internally, `faninany` uses the POSIX `select()` system call
to wait until traffic is available.

* literal

Use the `literal` stage to insert a line of literal text into a stream.

* cp

Use the `cp` stage to issue VM (CP) commands and recover their output.

This stage requires the z/VM hypervisor.
It really only works on z/Linux when hosted by z/VM.

This stage requires privileges. Usually, one must be root to issue CP commands.

* filer, aliased as "&lt;"

Use `&lt;` to read from a file.

* filew, aliased as "&gt;"

Use `&gt;` to write to a file.

* locate

Use the `locate` stage to find occurrences of the specified string in the input stream.

* strliteral

Use the `strliteral` stage to insert a line of literal text into a stream.



# Ductwork Protocol

This page describes the traffic flow over POSIX pipes.

## Ductwork Consumer/Producer Handshake

This implementation uses a pair of POSIX pipes
to effect the managed flow of data from one stage to the next.

When the producer (upstream) stage performs a write,
it blocks until the consumer (downstream) is able to ingest
the record.

## Ductwork Connector

The connection between stages is represented in source code by a C struct.


                        producer            consumer
                            ctrl <--------- ctrl
                            data ---------> data
                     mode=OUTPUT            mode=INPUT


## Ductwork Protocol

When the producer stage attempts to write a record,
it blocks on a POSIX read awaiting instructions from the
consumer stage. The consumer sends one of the following signals.

* `STAT`

When the consumer sends "STAT" on the control pipe,
the producer must send a description of the waiting record.

the only meta data at this point in the development

producer sends "DATA bytes seq"

* `PEEK`

think PIPLOCAT to examine a record

producer sends data

* `NEXT`

think PIPINPUT (sort of) consume the record

producer sends "OKAY" and advances the sequence count

* `QUIT`

for SEVER operation
          producer sends "`OKAY`"

* FAIL errorcode

if something went wrong

## Example Write and Read of one Record

The producer calls output(,srcbuf,) to write a record.

The consumer calls readto(,dstbuf,) to read a record.

                          producer            consumer
                      read(ctrl,,) <--------- write(ctrl,"STAT",)
     write(data,"DATA bytes seq",) ---------> read(data,,)
                      read(ctrl,,) <--------- write(ctrl,"PEEK",)
          write(data,srcbuf,bytes) ---------> read(data,dstbuf,bytes)
                      read(ctrl,,) <--------- write(ctrl,"NEXT",)





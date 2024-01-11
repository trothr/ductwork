# Ductwork Functions

This page lists some functions of the Ductwork library.

These are native functions, expressed here in C, but callable from any language which compiles to native machine code.
These are functions that the stages call.

`&pd` is a pointer to a "pipe descriptor" struct.
Any practical stage will have at least one input PD or output PD.
Most stages will have both input and output and may have multiples of either or both.

`buffer` is a pointer to a record data buffer.

`buflen` is an integer indicating the size of the buffer (for input)
or the length of the record (for output). A positive return code
indicates the number of bytes in the record (for input) or the number
of bytes successfully written (for output).

`&ps` is pointer to a master pipeline struct. When a stage starts,
it must call `pipeinit()` to populate this struct, which will then
reference a number of `pd` structs.

## Ductwork Functions

The following functions are available for 

* peekto

Use the `peekto()` fuction to examine an input record without consuming it.

    rc = xfl_peekto(&pd,buffer,buflen);

The pipe descriptor struct `pd` must be an input connector.

The return code will indicate the actual numberof bytes in the record.
A negative return code indicates an error.

`peekto()` is intended to work like the POSIX system `read()` function.
`peekto()` is inspired by the 'PEEKTO' command for Rexx based stages.

* readto

Use the `readto()` fuction to read (or just consume) an input record.

    rc = xfl_readto(&pd,buffer,buflen);

The pipe descriptor struct `pd` must be an input connector.

If `buffer` or `buflen` are NULL
or if the size indicated by `buflen` is zero,
then `readto()` will consume the record without saving it to the buffer.

Note that records can have zero length.
To read a zero-length record, supply a `buflen` of at least 1.
The return code will indicate the actual numberof bytes in the record.
A negative return code indicates an error.

`readto()` is intended to work like the POSIX system `read()` function.
`readto()` is inspired by the 'READTO' command for Rexx based stages.

* output

Use the `output()` function to write a record.
`output()` blocks until the record is consumed.

    rc = xfl_output(&pd,buffer,buflen);

The pipe descriptor struct `pd` must be an output connector.

The return code will indicate the actual numberof bytes written.
A negative return code indicates an error.

`output()` is intended to work like the POSIX system `write()` function.
`output()` is inspired by the 'OUTPUT' command for Rexx based stages.

* streamstate

Use the `streamstate()` function to interrogate the state of a stream.

    rc = xfl_streamstate(&pd);

* sever

Use the `sever()` function to disconnect a stream.

    rc = xfl_sever(&pd);

* addpipe

Use the `addpipe()` fuction to add a new pipeline to an existing stage.

    rc = xfl_addpipe(pipespec);

* addstream

Use the `addstream()` function to add a new stream to an running pipeline.

    rc = xfl_addstream(pipespec);

* callpipe

Use the `callpipe()` function to call a subordinate pipeline.

    rc = xfl_callpipe(pipespec);

* runpipe

Use the `runpipe()` function to launch a supplemental pipeline.

    rc = xfl_runpipe(pipespec);

* pipeinit

Use the `pipeinit()` function to initialize a stage.

This is mandatory.
Stages are launched by the operating system and will need structures
allocated and populated with essential information about the pipeline
they're running in.

    rc = xfl_stagestart(&ps);

`pipeinit()` takes one argument, a pointer to a master pipeline struct.



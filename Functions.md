# Ductwork Functions

This page lists some functions of the Ductwork library.

These are native functions, expressed here in C, but callable from any language which compiles to native machine code.
These are functions that the stages call.

`&pd` is a pointer to a "pipe descriptor" struct.
Any practical stage will have at least one input PD or output PD.
Most stages will have both input and output and may have multiples of either or both.

'buffer' is a pointer to a record data buffer.

`buflen` is to an integer indicating the length of the record (for output)
or the size of the buffer (for input). On return, input functions will have replaced
the size available with the actual size of the record.

## Ductwork Functions

The following functions are available for 

* peekto

Use the `peekto()` fuction to examine an input record without consuming it.

    rc = peekto(&pd,buffer,buflen);

The pipe descriptor struct `pd` must be an input connector.

* readto

Use the `readto()` fuction to read (or just consume) an input record.

    rc = readto(&pd,buffer,buflen);

The pipe descriptor struct `pd` must be an input connector.

If `buffer` or `buflen` are NULL
or if the size indicated by `buflen` is zero,
then `readto()` will consume the record without saving it to the buffer.

Note that records can have zero length.

* output

Use the `output()` function to write a record.
`output()` blocks until the record is consumed.

    rc = output(&pd,buffer,buflen);

The pipe descriptor struct `pd` must be an output connector.

* streamstate

Use the `streamstate()` function to interrogate the state of a stream.

    rc = streamstate(&pd);

* sever

Use the `sever()` function to disconnect a stream.

    rc = sever(&pd);

* addpipe

    rc = addpipe(pipespec);

* addstream

    rc = addstream(pipespec);

* callpipe

    rc = callpipe(pipespec);

* runpipe

    rc = runpipe(pipespec);

* pipeinit

    rc = pipeinit();

`pipeinit()` takes no arguments.



# Ductwork Functions

This page lists core functions of the Ductwork library.

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

* addstream

* callpipe

* runpipe







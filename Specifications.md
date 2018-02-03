# Ductwork Specifications

This document outlines the design specification of
the DUCTWORK project and enumerates the essential features.

## Ductwork Design

Ductwork is a pipeline environment supporting "flow programming".
It is inspired by CMS/TSO Pipelines and is intended to support
a subset of the capability and syntax of that implementation.

Syntaxtically, a Ductwork pipeline looks like a shell pipeline.

Conceptually, records in a Ductwork pipeline are similar to lines of text
in a plaintext file or stream, such as program source.

* independent stages

Pipelines are composed of independent programs called stages.
Traffic flows through the pipeline moving from the output of one stage
to the input of another stage.

* packetized blow (bounded messages, record-oriented)

Records (or packets, or messages) flow through the pipeline
as whole entities. A record may correspond to a line of text
but traditional Unix newline interpolation is opaque.
(A record may contain any number of newline characters
without affecting the structure of the record.)

* peek first, then consume

A record can be examined without being consumed.
This allows changes of flow based on content.

A stage can "peek" at an input record and deliver it (or its effects)
downstream before actually consuming the input record. In CMS/TSO Pipelines
a stage with this characteristic is said to "not delay the record".
To delay the record (or not) has a significant effect on traffic flow.



## Differences from CMS/TSO Pipelines

Ductwork uses the operating system kernel for scheduling.
It does not have its own dispatcher. This differs from CMS/TSO Pipelines
which has a dispatcher independent from the operating system dispatcher.

CMS/TSO Pipelines concepts such as commit levels are not supported in Ductwork.

## Differences from Shell Pipes



## Differences from Named Pipes


## Differences from Named Sockets







## Differences from TCP Sockets

TCP sockets explicitly do not guarantee message boundaries.
Bytes (octets) are assured to arrive in order, but traffic may be
quantized arbitrarily from source endpoint to target endpoint.













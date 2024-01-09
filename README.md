# Ductwork

a utility for flow-based programming

Ductwork provides on POSIX systems the same functionality
as CMS/TSO Pipelines provides on IBM mainframe systems CMS and TSO.

## Goals

Goals of the Ductwork project:

* simulate CMS/TSO Pipelines for POSIX environments
* allow stages to run independently alongside other programs
* impose minimal infrastructure and environmental requirements
* provide as many pre-defined stages as practical
* allow custom stages in any language

Each "stage" is a separate process.

Stages can be written in any language.

The "dispatcher" arranges file descriptors and spawns the stages.
It then lingers, much like the shell does when running multiple
stages (but the plumbing here is stronger), then exits when all
sub-processes have exited.

Hereafter we refer to it as the "launcher" because it is
not a dispatcher in the sense of scheduling threads.
That job is left to the operating system kernel.

## Ductwork

Ductwork is a CMS/TSO Pipelines work-alike for POSIX systems,
providing the same basic functionality as that of of CMS/TSO Pipelines,
but in a POSIX environment where IBM VM/CMS and IBM MVS/TSO are not available.

Ductwork is compatible, as much as is possible, with CMS Pipelines
at the command-line level.  Dispatching is handled by the underlying
operating system (Unix, Linux, Windows).  The difference in dispatching
is a major functional difference between Ductwork and CMS Pipelines.

Ductwork accepts traditional Unix style options (with dashes)
but also accepts traditional CMS style options (in parentheses).

Ductwork does not require a closed system such as containment within a
Java virtual machine (JVM).

Too many technologies, services, systems, environments, projects,
and libraries refer to "pipelines", so we use the name "ductwork"
in hopes of conveying the full idea. If that doesn't work,
maybe we'll call this thing "plenum".

## Overview

Ductwork provides a command, '`pipe`'. The argument string to the
'`pipe`' command is the pipeline specification. '`pipe'` selects
programs to run and chains them together in a pipeline to pump data through.

Ductwork pipelines do not involve the usual POSIX stdin and stdout interface.
The chaining of Ductwork pipelines is more robust, bounded out of band,
and flow controlled. Ductwork has a library of built-in programs that
can be called in a pipeline specification. These built-in programs
interface with the operating system, and perform many utility functions.

Data on CMS is structured in logical records (or "packets" or "messages")
rather than a stream of bytes. For textual data, a line of text corresponds
to a logical record without a newline marker character. The data is passed
by Ductwork between the stages as logical records.

Ductwork users issue pipeline commands from the terminal, or in shell scripts,
or in any standard programming environment. Users can write Ductwork "stage"
programs in any language which compiles to native or in any interpreted
language which can call the support library. This includes the popular
Rexx language common on CMS and TSO. Home grown stages supplement
the built-in Ductwork programs.

This document does not attempt to explain CMS Pipelines or TSO Pipelines.
For general information about CMS/TSO Pipelines, see the web page

https://en.wikipedia.org/wiki/CMS_Pipelines

## Additional Links

http://www.casita.net/pub/ductwork/

http://vm.marist.edu/~pipeline/

<!-- http://code.google.com/p/ductwork/ -->

https://www.youtube.com/watch?v=AUsCdmjlaSU

http://www.rvdheij.nl/pdweb/

## Contributing

Here's
how we suggest you go about proposing a change to this project:

1. Fork this project to your account.
1. Create a branch for the change you intend to make.
1. Make your changes to your fork.
1. Send a pull request from your
fork's
branch to our master branch.

Using the web-based interface to make changes is fine too,
and will help you by automatically forking the project
and prompting to send a pull request too.



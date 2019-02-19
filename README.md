# ductwork
a utility for flow-based programming

Ductwork aims to provide on POSIX systems the same functionality
as CMS Pipelines provides on IBM mainframe systems CMS and TSO.

## goals

Goals of the Ductwork project:

* simulate CMS/TSO Pipelines for POSIX environments
* allow stages to run independently
* impose minimal infrastructure and environmental requirements
* provide as many pre-defined stages as practical
* allow custom stages in any language

## ductwork

Ductwork is a CMS Pipelines work-alike for POSIX systems
providing the same basic functionality as that of of CMS/TSO Pipelines
but in a POSIX environment where IBM VM/CMS and IBM MVS/TSO are not available.

Ductwork is compatible with CMS Pipelines at the command-line level
as much as is possible.  Dispatching is handled by the underlying
operating system (Unix, Linux, Windows, or even OpenVM).
Ductwork accepts traditional Unix style options
but also accepts traditional CMS and MVS style options.

Too many technologies, services, systems, environments, projects, and libraries
refer to "pipelines", so we use the name "ductwork" in hopes of conveying
the same idea. If that doesn't work, maybe we'll call this thing "plenum".

This document does not explain CMS Pipelines at all.  For general
information about CMS Pipelines, see the web page



## links

https://en.wikipedia.org/wiki/CMS_Pipelines

http://vm.marist.edu/~pipeline/

http://code.google.com/p/ductwork/

https://www.youtube.com/watch?v=AUsCdmjlaSU

http://www.rvdheij.nl/pdweb/


## contributing

Here’s how we suggest you go about proposing a change to this project:

1. Fork this project to your account.
1. Create a branch for the change you intend to make.
1. Make your changes to your fork.
1. Send a pull request from your fork’s branch to our master branch.

Using the web-based interface to make changes is fine too, and will help you by automatically forking the project and prompting to send a pull request too.










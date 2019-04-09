# Ductwork Parser

This page describes the pipeline parsing in Ductwork.

When a pipeline is instantiated from a shell,
care must be taken that the pipeline specification be quoted.
This is especially important for the stage separator so that it
will be interpreted by the parser and not by the shell.

## Stage Separator

The default stage separator character is \|.

Stages in Ductwork, as with CMS/TSO Pipelines,
are distinguished by a separator character. By default,
this character is a vertical bar so that Ductwork pipelines
look visually similar to shell pipelines.

    --stagesep
    --separator

## Pipeline Separator (Endchar)

The default pipeline separator character is \!.

Multiple simultaneous streams can be defined, typically with
connections between pipelines. (Not strictly required.)
Stream definitions are distinquished by the pipeline separator.

    --endchar

## Command Options

The main Ductwork command allows options to be specified using
VM/CMS style, for nominal compatibility with CMS/TSOPipelines,
or using Unix style as is somewhat easier on other systems.













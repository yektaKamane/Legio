# Legio

Library to introduce fault-tolerance in MPI in the form of graceful degradation

> We are led on by courage, and obedience, and fortitude, which [...] does not desert us in our ill fortune.
>
> -- <cite>Titus Flavius Josephus</cite>

## Overview

Legio is a library that introduces fault-tolerance in MPI applications in the form of graceful degradation. It's designed for embarrassingly parallel applications. It is based on [ULFM](https://fault-tolerance.org/2017/11/03/ulfm-2-0/).

## Usage

One of the key aspects of Legio is the transparency of integration: no changes in the code are needed, integration is performed via linking. Legio leverages PMPI to catch all the calls toward MPI and wraps them with the appropriate code needed.
If you have ULFM already installed, run:

    $ export ULFM_PREFIX <path-to-ulfm-build-folder>

To compile your application with Legio, put the sources inside the /src folder, then run

    $ make all

To run the application linked with Legio, run

    $ make run

## MPI functions supported

The list of the functions supported can be found in [this file](./calls_support.csv), together with an analysis on the difficulty of integration for the non-supported ones and a brief note on what should be done.

Support for other calls is under development.

## Configuration

Developed, documentation incoming.

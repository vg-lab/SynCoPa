# SynCoPa
(c) 2015-2016. GMRV / URJC

www.gmrv.es
gmrv@gmrv.es

## Introduction

ToDo

## Dependencies

### Strong dependences:

ToDo

### Weak dependences

ToDo

(*) Note: In order to connect applications one another, it is necessary to 
compile the project with ZeroEQ and its vocabulary libraries.

## Building

Syncopa has been succesfully built and used on Ubuntu 14.04/16.04, Mac OSX
Yosemite and Windows 7/8 (Visual Studio 2013 Win64). The following steps
should be enough to build it:

```bash
git clone https://gitlab.gmrv.es/nsviz/syncopa.git syncopa
mkdir syncopa/build && cd syncopa/build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

#!/bin/sh

CURDIR=`pwd`

cd LLiPmDRHachi/LLiPm.linuxproj


make DRP208

cp Obj/*.a ../../lib/


make DRC225

cp Obj/*.a ../../lib/


make DRP215

cp Obj/*.a ../../lib/


make DRC240

cp Obj/*.a ../../lib/

cd $CURDIR

ls -la ./lib

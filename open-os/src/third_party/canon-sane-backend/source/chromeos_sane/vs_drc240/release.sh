#!/bin/sh

CURDIR=`pwd`

cd prj_linux


make

cp vs.so ../../../files/drc230vs.so
cp vs.so ../../../files/drc240vs.so
cp vs.so ../../../files/r40vs.so

cd $CURDIR
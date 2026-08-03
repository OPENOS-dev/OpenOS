#!/bin/sh

CURDIR=`pwd`

cd prj_linux


make

cp vs.so ../../../files/drc225iivs.so

cd $CURDIR
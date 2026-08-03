#!/bin/sh

CURDIR=`pwd`

cd prj_linux


make

cp vs.so ../../../files/drm260vs.so
cp vs.so ../../../files/r50vs.so

cd $CURDIR
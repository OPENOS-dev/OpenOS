#!/bin/sh

CURDIR=`pwd`

echo "rm lib/*.a"
rm lib/*.a

echo ""
echo "SANESDK"

cd prj_linux


make

cd $CURDIR

echo ""
echo ""
echo ""
ls -l lib/

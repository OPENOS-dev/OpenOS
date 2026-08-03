#!/bin/sh

CURDIR=`pwd`

echo "rm lib/*.a"
rm lib/*.a

echo "VSSDK"

cd prj_vssdk/prj_linux


make

cd $CURDIR

echo ""
echo ""
echo "CSDSDK"

cd prj_csdsdk/prj_linux


make

cd $CURDIR

echo ""
echo ""
echo "IPSDK"

cd prj_ipsdk/prj_linux


make

cd $CURDIR

echo ""
echo ""
echo ""
ls -l lib/*.a

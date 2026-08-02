#!/bin/bash

CURDIR=`pwd`

cd $CURDIR

SUBPROJECTS=(
    ceiusb
    csdcore/vscsdsdk
    sanedriver
)

for ((i =0; i < ${#SUBPROJECTS[@]}; ++i))
do
	cd $CURDIR
	cd ${SUBPROJECTS[$i]}
    ./release.sh
done

cd $CURDIR

cp /usr/local/lib/libjpeg.so ./files/
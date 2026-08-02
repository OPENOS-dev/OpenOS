#!/bin/sh

CURDIR=`pwd`
SANESRCDIR='sane-backends-1.0.32'
SANESRCTAR=$SANESRCDIR'.tar.gz'

rm -rf $SANESRCDIR

tar xvfz $SANESRCTAR

mv $SANESRCDIR sane-backends

cd sanesdk

./release.sh

cd $CURDIR






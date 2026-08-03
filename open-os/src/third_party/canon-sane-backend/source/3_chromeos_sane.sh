#!/bin/bash

CURDIR=`pwd`

cd $CURDIR

./release.sh

cd $CURDIR

cd chromeos_sane

./release.sh

cd $CURDIR

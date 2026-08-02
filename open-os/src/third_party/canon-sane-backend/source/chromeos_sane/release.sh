#!/bin/sh

CURDIR=`pwd`
SANESRCDIR='sane-backends-1.0.32'
SANESRCTAR=$SANESRCDIR'.tar.gz'
VERSIONNUMBER=$1

rm -rf $SANESRCDIR
rm -rf vscsdsdk
rm -rf sanesdk
rm -rf files
rm -rf sane-backends

mkdir vscsdsdk
mkdir sanesdk

cp -r ../csdcore/vscsdsdk/include ./vscsdsdk/
cp -r ../csdcore/vscsdsdk/lib ./vscsdsdk/
cp -r ../sanedriver/sanesdk/include ./sanesdk/
cp -r ../sanedriver/sanesdk/lib ./sanesdk/

cp ../sanedriver/*.tar.gz .

mkdir files

tar xvfz $SANESRCTAR

rm *.tar.gz

mv $SANESRCDIR sane-backends

cd $CURDIR

echo "make CsdCore.so"

cd csdcore/prj_linux


make

cp CsdCore.so ../../../files

cd $CURDIR

echo "adjust sdk"

cd adjustsdk

./release.sh

cd $CURDIR

echo "make vs.so"

cd vs_drm260

./release.sh

cd $CURDIR

cd vs_p208ii

./release.sh

cd $CURDIR

cd vs_p215ii

./release.sh

cd $CURDIR

cd vs_drc225ii

./release.sh drc225ii

cd $CURDIR

cd vs_drc240

./release.sh

cd $CURDIR

echo "make sane"

cd sane/prj_linux


make clean
make SCANNER=drm260

cp *.so* ../../../files


make clean
make SCANNER=p208ii

cp *.so* ../../../files


make clean
make SCANNER=drp208ii

cp *.so* ../../../files


make clean
make SCANNER=p215ii

cp *.so* ../../../files


make clean
make SCANNER=drp215ii

cp *.so* ../../../files


make clean
make SCANNER=drc225ii

cp *.so* ../../../files


make clean
make SCANNER=drc230

cp *.so* ../../../files


make clean
make SCANNER=drc240

cp *.so* ../../../files


make clean
make SCANNER=r40

cp *.so* ../../../files


make clean
make SCANNER=r50

cp *.so* ../../../files

cd $CURDIR

cp ../files/drc240.ini ../files/drc230.ini
cp ../files/drc240.ini ../files/r40.ini
cp ../files/p208ii.ini ../files/drp208ii.ini
cp ../files/p215ii.ini ../files/drp215ii.ini

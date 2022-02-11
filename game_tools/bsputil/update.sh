#!/bin/sh

set -o errexit

cd "${0%/*}"

for SUBDIR in src/common src/light tmp; do
	if [ -e ${SUBDIR} ]; then
		rm -r ${SUBDIR}
	fi
done

mkdir -p src/common src/light tmp

COMMIT=6e96ae7cc21b1405b3f5b37389383449f17b6a9d

cd tmp
curl -JOL https://github.com/ericwa/ericw-tools/archive/${COMMIT}.zip
tar -xf ericw-tools-${COMMIT}.zip

cd ericw-tools-${COMMIT}/common
mv bspfile.cc bsputils.cc cmdlib.cc entdata.cc log.cc mathlib.cc polylib.cc threads.cc ../../../src/common

cd ../include/common
mv entdata.h bspfile.hh bsputils.hh cmdlib.hh log.hh mathlib.hh polylib.hh qvec.hh threads.hh ../../../../src/common

cd ../light
mv entities.hh imglib.hh light.hh litfile.hh ltface.hh settings.hh trace.hh ../../../../src/light

cd ../../bsputil
mv bsputil.cc decompile.cpp decompile.h ../../../src

cd ../..
rm -r ericw-tools-${COMMIT} ericw-tools-${COMMIT}.zip

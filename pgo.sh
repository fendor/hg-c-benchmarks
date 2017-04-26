#!/usr/bin/env bash
#SANITY
set -ue

#CONFIG
BINARY=Nbody-OpenMP

#CODE
rm -rf build/
mkdir build
cd build
echo Building
cmake -DCMAKE_C_FLAGS="-g -fprofile-generate" ..
make
echo Recording profile
sudo perf record -b ./$BINARY 1000 1000
echo Recompiling
make
echo "DONE"

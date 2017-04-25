#!/usr/bin/env bash
#SANITY
set -ue

#CONFIG
BINARY=hg-c-benchmark

#CODE
rm -r build/ 2>/dev/null || true
mkdir build 2>/dev/null || true
cd build
cmake -DCMAKE_C_FLAGS="-g -fprofile-instr-generate" ..
make
perf record -b ./$(BINARY)
create_llvm_prof --binary=./$(BINARY) --out=$(BINARY).prof
cmake -DCMAKE_C_FLAGS="-fprofile-sample-use=code.prof" ..
make
echo "DONE"

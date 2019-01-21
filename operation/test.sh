#!/usr/bin/bash

echo "--> Generating"
python3 data/generate.py --rows 1000000 --cols 50

echo ""
echo "--> Python"
python3 python/operation.py

cd c
echo ""
make

echo ""
echo "--> C: 1 thread"
OMP_NUM_THREADS=1 ./operation

echo ""
echo "--> C: 2 thread"
OMP_NUM_THREADS=2 ./operation

echo ""
echo "--> C: 3 thread"
OMP_NUM_THREADS=3 ./operation

echo ""
echo "--> C: 4 thread"
OMP_NUM_THREADS=4 ./operation

cd ..

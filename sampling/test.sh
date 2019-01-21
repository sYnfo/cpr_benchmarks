#!/usr/bin/bash

echo "--> Python: Numpy"
python3 python/sampling.py --rows 1000000 --cols 20

echo ""
echo "--> Python: LCG"
python3 python/sampling.py --rows 1000000 --cols 20 --use-lcg

cd c
echo ""
make

echo ""
echo "--> C: GSL: 1 thread"
OMP_NUM_THREADS=1 ./sampling --rows 1000000 --cols 20

echo ""
echo "--> C: GSL: 2 thread"
OMP_NUM_THREADS=2 ./sampling --rows 1000000 --cols 20

echo ""
echo "--> C: GSL: 3 thread"
OMP_NUM_THREADS=3 ./sampling --rows 1000000 --cols 20

echo ""
echo "--> C: GSL: 4 thread"
OMP_NUM_THREADS=4 ./sampling --rows 1000000 --cols 20

echo ""
echo "--> C: LCG: 1 thread"
OMP_NUM_THREADS=1 ./sampling --rows 1000000 --cols 20 --use-lcg

echo ""
echo "--> C: LCG: 2 thread"
OMP_NUM_THREADS=2 ./sampling --rows 1000000 --cols 20 --use-lcg

echo ""
echo "--> C: LCG: 3 thread"
OMP_NUM_THREADS=3 ./sampling --rows 1000000 --cols 20 --use-lcg

echo ""
echo "--> C: LCG: 4 thread"
OMP_NUM_THREADS=4 ./sampling --rows 1000000 --cols 20 --use-lcg

cd ..

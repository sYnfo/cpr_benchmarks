#!/usr/bin/bash

echo "--> Generating"
python3 data/generate.py --m 20 --k 100

echo ""
echo "--> Python"
python3 python/matrix_vect_multiple.py --iterations 1000000

cd c
echo ""
make

echo ""
echo "--> C: 1 thread"
OMP_NUM_THREADS=1 ./matrix_vect_multiple --iteration 1000000

echo ""
echo "--> C: 2 thread"
OMP_NUM_THREADS=2 ./matrix_vect_multiple --iteration 1000000

echo ""
echo "--> C: 3 thread"
OMP_NUM_THREADS=3 ./matrix_vect_multiple --iteration 1000000

echo ""
echo "--> C: 4 thread"
OMP_NUM_THREADS=4 ./matrix_vect_multiple --iteration 1000000

cd ..

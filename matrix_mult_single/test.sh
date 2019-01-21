#!/usr/bin/bash

echo "--> Generating"
python3 data/generate.py --m 10000 --k 10000 --n 1000

echo ""
echo "--> Python"
python3 python/matrix_mult_single.py

cd c
echo ""
make

echo ""
echo "--> C: 1 thread"
OMP_NUM_THREADS=1 ./matrix_mult_single

echo ""
echo "--> C: 2 thread"
OMP_NUM_THREADS=2 ./matrix_mult_single

echo ""
echo "--> C: 3 thread"
OMP_NUM_THREADS=3 ./matrix_mult_single

echo ""
echo "--> C: 4 thread"
OMP_NUM_THREADS=4 ./matrix_mult_single

cd ..

echo ""
echo "Performance should vary drastically across machines. Note Numpy always \
uses full cores."

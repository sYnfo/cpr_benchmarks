#!/usr/bin/bash

echo "--> Generating"
python3 data/generate.py --batches 1000

echo ""
echo "--> Python: 1 simulation"
python3 python/simulation.py --simulations 1

cd c
echo ""
make

echo ""
echo "--> C: 1000 simulations: 1 thread"
OMP_NUM_THREADS=1 ./simulation --simulations 1000

echo ""
echo "--> C: 1000 simulations: 2 thread"
OMP_NUM_THREADS=2 ./simulation --simulations 1000

echo ""
echo "--> C: 1000 simulations: 3 thread"
OMP_NUM_THREADS=3 ./simulation --simulations 1000

echo ""
echo "--> C: 1000 simulations: 4 thread"
OMP_NUM_THREADS=4 ./simulation --simulations 1000

cd ..

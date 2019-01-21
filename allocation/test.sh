#!/usr/bin/bash

echo "--> Generating: RAM (/dev/shm)"
python3 data/generate.py --rows 1000000 --cols 5 --use-ram

echo ""
echo "--> Python: RAM (/dev/shm)"
python3 python/allocation.py --use-ram

cd c
echo ""
make

echo ""
echo "--> C: RAM (/dev/shm)"
./allocation --use-ram
cd ..

echo ""
echo "--> Generating: No RAM"
python3 data/generate.py --rows 1000000 --cols 5

echo ""
echo "--> Python: No RAM"
python3 python/allocation.py

echo ""
echo "--> C: No RAM"
cd c
./allocation
cd ..

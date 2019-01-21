# Simulation
For many simulations, updates a state until it has reached an absorbing value
and writes the value/score to memory. Distributions are calculated using
softmax.

Note to test the outputs of Python vs C, use

    python3 data/generate.py
    cd c
    OMP_NUM_THREADS=1 ./simulation --save --use-lcg
    python3 ../python/simulation.py --save --use-lcg
    cat ../data/home_c.csv
    cat ../data/home_py.csv
    cat ../data/away_c.csv
    cat ../data/away_py.csv
    cd ..

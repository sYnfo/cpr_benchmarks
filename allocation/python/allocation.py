"""Loads the data, allocates to a an array (happens automatically) then outputs
to a new csv file."""

import os

import numpy as np
from contexttimer import Timer
import click

@click.command()
@click.option('--use-ram', is_flag=True, help='Save data to row.')
def allocation(use_ram):
    # Sort out where to load data from.
    if use_ram:
        # Load from /dev/shm.
        dir_path = '/dev/shm/allocation/data'
        if not os.path.exists(dir_path):
            raise Exception("Path doesn't exist %s" % dir_path)
    else:
        dir_path = os.path.dirname(os.path.abspath(__file__))

    # Load and allocate data to an array.
    fname_csv = os.path.split(dir_path)[0] + '/data/data.csv'
    fname_bin = os.path.split(dir_path)[0] + '/data/data.bin'
    fname_meta_bin = os.path.split(dir_path)[0] + '/data/data_meta.bin'

    with Timer() as t:
        data_bin = np.fromfile(fname_bin, np.float64)
        rows, cols = np.fromfile(fname_meta_bin, np.int)
        data_bin = data_bin.reshape([rows, cols])  # Only a view so should be fast.
        print("LOAD_ALLOCATE_BIN: %s." % t.elapsed)

    with Timer() as t:
        data_csv = np.loadtxt(fname_csv, np.float64, delimiter=',')
        print("LOAD_ALLOCATE_CSV: %s." % t.elapsed)

    assert ((data_bin - data_csv)**2 < 1e-6).all()

    # Save data to csv for comparison.
    with Timer() as t:
        np.savetxt(os.path.split(dir_path)[0] + '/data/data_py.csv', data_csv,
                   delimiter=',', fmt='%.10f')
        print("SAVE_CSV: %s." % t.elapsed)

if __name__ == "__main__":
    allocation()

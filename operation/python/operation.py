"""Loads the data, allocates to a an array (happens automatically) then outputs
to a new csv file."""

import os

import numpy as np
from contexttimer import Timer
import click

@click.command()
@click.option('--save', is_flag=True, help='Whether to save result to csv.')
def operation(save):
    dir_path = os.path.dirname(os.path.abspath(__file__))

    # Load and allocate data to an array.
    fname_bin = os.path.split(dir_path)[0] + '/data/data.bin'
    fname_meta_bin = os.path.split(dir_path)[0] + '/data/data_meta.bin'

    with Timer() as t:
        data_bin = np.fromfile(fname_bin, np.float64)
        rows, cols = np.fromfile(fname_meta_bin, np.int)
        data_bin = data_bin.reshape([rows, cols])  # Only a view so should be fast.
        print("LOAD_ALLOCATE_BIN: %s." % t.elapsed)

    with Timer() as t:
        np.exp(data_bin, data_bin)
        print("EXP: %s." % t.elapsed)

    # Save data to csv for comparison.
    if save:
        with Timer() as t:
            np.savetxt(os.path.split(dir_path)[0] + '/data/data_py.csv', data_bin,
                       delimiter=',', fmt='%.10f')
            print("SAVE_CSV: %s." % t.elapsed)

if __name__ == "__main__":
    operation()

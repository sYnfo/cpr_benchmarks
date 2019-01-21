"""Loads the data, allocates to a an array (happens automatically) then outputs
to a new csv file."""

import os

import numpy as np
from contexttimer import Timer
import click

@click.command()
@click.option('--save', is_flag=True, help='Whether to save result to csv.')
def matrix_mult_single(save):
    dir_path = os.path.dirname(os.path.abspath(__file__))

    # Load and allocate data to an array.
    fname_A_bin = os.path.split(dir_path)[0] + '/data/A.bin'
    fname_B_bin = os.path.split(dir_path)[0] + '/data/B.bin'
    fname_meta_bin = os.path.split(dir_path)[0] + '/data/meta.bin'

    with Timer() as t:
        A = np.fromfile(fname_A_bin, np.float64)
        B = np.fromfile(fname_B_bin, np.float64)
        m, k, n = np.fromfile(fname_meta_bin, np.int)

        A = A.reshape([m, k])  # Only a view so should be fast.
        B = B.reshape([k, n])  # Only a view so should be fast.
        print("LOAD_ALLOCATE_BIN: %s." % t.elapsed)

    with Timer() as t:
        C = np.empty([m, n], dtype=np.float64)
        np.matmul(A, B, out=C)
        print("MATRIX_MULT_SINGLE: %s." % t.elapsed)

    # Save data to csv for comparison.
    if save:
        with Timer() as t:
            np.savetxt(os.path.split(dir_path)[0] + '/data/C_py.csv', C,
                       delimiter=',', fmt='%.10f')
            print("SAVE_CSV: %s." % t.elapsed)

if __name__ == "__main__":
    matrix_mult_single()

"""Loads the data, allocates to a an array (happens automatically) then outputs
to a new csv file."""

import os

import numpy as np
from contexttimer import Timer
import click

@click.command()
@click.option('--iterations', default=1000, help='Total matrix vector multiplications to perform.')
@click.option('--save', is_flag=True, help='Whether to save result to csv.')
def matrix_vector_multiple(iterations, save):
    dir_path = os.path.dirname(os.path.abspath(__file__))

    # Load and allocate data to an array.
    fname_A_bin = os.path.split(dir_path)[0] + '/data/A.bin'
    fname_B_bin = os.path.split(dir_path)[0] + '/data/B.bin'
    fname_meta_bin = os.path.split(dir_path)[0] + '/data/meta.bin'

    with Timer() as t:
        A = np.fromfile(fname_A_bin, np.float64)
        B = np.fromfile(fname_B_bin, np.float64)
        m, k = np.fromfile(fname_meta_bin, np.int)

        A = A.reshape([m, k])  # Only a view so should be fast.
        B = B.reshape([k, 1])  # Only a view so should be fast.
        print("LOAD_ALLOCATE_BIN: %s." % t.elapsed)

    # Just store the repeated iterations back in same object C.
    with Timer() as t:
        C = np.empty([m, iterations], dtype=np.float64)
        for i in range(iterations):
            #np.matmul(A, B, out=C[:, [i]])
            C[:, [i]] = np.matmul(A, B)
        print("MATRIX_VECT_MULTIPLE: %s." % t.elapsed)

    # Save data to csv for comparison.
    if save:
        with Timer() as t:
            np.savetxt(os.path.split(dir_path)[0] + '/data/C_py.csv',
                       C[:, np.random.randint(iterations)],
                       delimiter=',', fmt='%.10f')
            print("SAVE_CSV: %s." % t.elapsed)

if __name__ == "__main__":
    matrix_vector_multiple()

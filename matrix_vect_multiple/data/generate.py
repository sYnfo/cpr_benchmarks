"""Generate some float64 csv data and write to a file."""

import os

import numpy as np
from contexttimer import Timer
import click

@click.command(help="Generate matrix A and vector B to find A @ B.")
@click.option('--m', default=100, help='Total rows of A.')
@click.option('--k', default=5, help='Total columns of A (and rows of B).')
def generate(m, k):
    # Sort out where to save data.
    dir_path = os.path.dirname(os.path.abspath(__file__))

    # Generate data.
    with Timer() as t:
        A = 100*np.random.rand(m, k).astype(np.float64)
        B = 100*np.random.rand(k, 1).astype(np.float64)
        print("Time to generate data: %s." % t.elapsed)

    # Save binary data.
    with Timer() as t:
        A.tofile(dir_path + '/A.bin')
        B.tofile(dir_path + '/B.bin')
        np.array([m, k], dtype=np.int).tofile(dir_path + '/meta.bin')
        print("Time to save binary data: %s." % t.elapsed)


if __name__ == "__main__":
    generate()

"""Generate some float64 csv data and write to a file."""

import os

import numpy as np
from contexttimer import Timer
import click

@click.command()
@click.option('--rows', default=1000, help='Total rows.')
@click.option('--cols', default=5, help='Total columns.')
def generate(rows, cols):
    # Sort out where to save data.
    dir_path = os.path.dirname(os.path.abspath(__file__))

    # Generate data.
    with Timer() as t:
        data = 100*np.random.rand(rows, cols).astype(np.float64)
        print("Time to generate data: %s." % t.elapsed)

    # Save data binary data.
    with Timer() as t:
        data.tofile(dir_path + '/data.bin')
        np.array([rows, cols], dtype=np.int).tofile(dir_path + '/data_meta.bin')
        print("Time to save binary data: %s." % t.elapsed)

if __name__ == "__main__":
    generate()

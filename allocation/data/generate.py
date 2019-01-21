"""Generate some float64 csv data and write to a file."""

import os

import numpy as np
from contexttimer import Timer
import click

@click.command()
@click.option('--rows', default=100, help='Total rows.')
@click.option('--cols', default=5, help='Total columns.')
@click.option('--use-ram', is_flag=True, help='Save data to row.')
def generate(rows, cols, use_ram):
    # Sort out where to save data.
    if use_ram:
        # Save to /dev/shm.
        dir_path = '/dev/shm/allocation/data'

        if not os.path.exists(dir_path):
            os.makedirs(dir_path)
    else:
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

    # Save data csv data.
    with Timer() as t:
        np.savetxt(dir_path + '/data.csv', data, delimiter=',', fmt='%.10f')
        print("Time to save CSV data: %s." % t.elapsed)

if __name__ == "__main__":
    generate()

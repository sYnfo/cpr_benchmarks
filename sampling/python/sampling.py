"""Loads the data, allocates to a an array (happens automatically) then outputs
to a new csv file."""

import os

import numpy as np
from contexttimer import Timer
import click


class LCG():
    """A linear congruential generator class.
    See https://en.wikipedia.org/wiki/Linear_congruential_generator."""

    def __init__(self, a, c, m, seed):
        self.a = a
        self.c = c
        self.m = m
        self.seed = seed

    def rand_int(self):
        """Random integer on the range [0, self.m-1]."""
        self.seed = (self.seed*self.a + self.c) % self.m

        return self.seed

    def rand(self):
        """Uniform random number between [0,1]."""
        return self.rand_int()/(self.m - 1)


@click.command()
@click.option('--rows', default=10000, help='Total rows of simulation.')
@click.option('--cols', default=5, help='Total columns of simulation.')
@click.option('--use-lcg', is_flag=True, help='Use a LCG generator (otherwise Numpy).')
@click.option('--save', is_flag=True, help='Whether to save result to csv.')
def sampling(rows, cols, use_lcg, save):
    dir_path = os.path.dirname(os.path.abspath(__file__))

    # Create samples.
    with Timer() as t:
        if use_lcg:
            lcg = LCG(106, 1283, 6075, 42)
            data = np.empty([rows, cols])
            for i in range(rows):
                for j in range(cols):
                    data[i, j] = lcg.rand()
        else:
            data = np.random.rand(rows, cols)
        print("SAMPLE: %s." % t.elapsed)

    # Save data to csv for comparison.
    if save:
        with Timer() as t:
            np.savetxt(os.path.split(dir_path)[0] + '/data/data_py.csv', data,
                       delimiter=',', fmt='%.10f')
            print("SAVE_CSV: %s." % t.elapsed)

if __name__ == "__main__":
    sampling()

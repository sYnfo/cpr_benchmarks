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
@click.option('--simulations', default=10, help='Total simulations to perform for each batch.')
@click.option('--use-lcg', is_flag=True, help='Use a LCG generator (otherwise Numpy).')
@click.option('--save', is_flag=True, help='Whether to save result to csv.')
def simulation(simulations, use_lcg, save):
    dir_path = os.path.dirname(os.path.abspath(__file__))
    N_simulations = simulations

    # Load and allocate data to an array.
    fname_b_bin = os.path.split(dir_path)[0] + '/data/b.bin'
    fname_W_bin = os.path.split(dir_path)[0] + '/data/W.bin'
    fname_hidden_bin = os.path.split(dir_path)[0] + '/data/hidden.bin'
    fname_meta_bin = os.path.split(dir_path)[0] + '/data/meta.bin'

    with Timer() as t:
        b = np.fromfile(fname_b_bin, np.float64)
        W = np.fromfile(fname_W_bin, np.float64)
        hidden = np.fromfile(fname_hidden_bin, np.float64)
        N_batches, N_hidden, K, N_features = np.fromfile(fname_meta_bin, np.int)

        b = b.reshape([K, 1])  # Only a view so should be fast.
        W = W.reshape([K, N_features])  # Only a view so should be fast.
        hidden = hidden.reshape([N_batches, N_hidden])  # Only a view so should be fast.
        print("LOAD_ALLOCATE_BIN: %s." % t.elapsed)

    # Define the random number generator.
    lcg = LCG(106, 1283, 6075, 42)
    if use_lcg:
        sample = lambda: lcg.rand()
    else:
        sample = lambda: np.random.rand()

    # Apply the simulator.
    with Timer() as t:
        dist = np.empty([K, 1])
        result = np.empty([N_batches, N_simulations, 2])

        for i in range(N_batches * N_simulations):
            # Inner loop over batches to give most diverse output if cut short.
            simulation, batch = divmod(i, N_batches)

            # Declare state (maybe from here should be in separate function).
            state = {'home_score': 0,
                     'away_score': 0,
                     'total_events': 0}

            derived_state = {'score': None,
                             'is_home': None,
                             'is_winning': None,
                             'player_ability': None}

            while ((state['home_score'] < 10) and (state['away_score'] < 10)):
                # Get derived state.
                away = state['total_events'] % 2
                home = 1 - away

                player = state['total_events'] % 6

                derived_state['score'] = state['home_score']*home + \
                                         state['away_score']*away

                derived_state['is_home'] = 1 - away

                derived_state['is_winning'] = \
                    (state['home_score'] > state['away_score']) * home + \
                    (state['home_score'] < state['away_score']) * away

                derived_state['player_ability'] = hidden[batch, player]

                # Get the distributions.
                x = np.array(list(derived_state.values()))

                dist[:] = b
                dist[:] = np.exp(dist + np.matmul(W, x)[:, None])

                dist_sum = dist.sum()

                # Simulate event from distribution.
                u = dist_sum*sample()
                dist_cum = 0
                outcome = K - 1
                for j in range(K - 1):
                    dist_cum += dist[j]
                    if u < dist_cum:
                        outcome = j

                # Apply the outcome to the state.
                state['total_events'] += 1

                state['home_score'] += home*outcome
                state['away_score'] += away*outcome

            # Match has ended so fill in scores into the result array.
            result[batch, simulation, 0] = state['home_score']
            result[batch, simulation, 1] = state['away_score']


        print("SIMULATION: %s." % t.elapsed)

    # Save data to csv for comparison.
    if save:
        with Timer() as t:
            np.savetxt(os.path.split(dir_path)[0] + '/data/home_py.csv',
                       result[:, :, 0],
                       delimiter=',', fmt='%d')

            np.savetxt(os.path.split(dir_path)[0] + '/data/away_py.csv',
                       result[:, :, 1],
                       delimiter=',', fmt='%d')
            print("SAVE_CSV: %s." % t.elapsed)

if __name__ == "__main__":
    simulation()

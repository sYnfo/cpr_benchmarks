"""Generate some float64 csv data and write to a file."""

import os

import numpy as np
from contexttimer import Timer
import click

@click.command(help="Generate objects to perform simulations.")
@click.option('--batches', default=10, help='Total rows of A.')
def generate(batches):
    """The simulator consists of the following objects.

    A state (all information for representing current position in game):
        home_score: Home team score.
        away_score: Away team score.
        total_events: Total events so far observed.

    A hidden feature matrix:
        matrix: A (N_batches, N_hidden) matrix representing the hidden feature
                values for each player (in the match) in each batch.

    A derived state (derived from state and hidden, containing features):
        score: Score for current team.
        is_home: Whether current event is for home team.
        is_winning: Is the their team winning at the event.
        player_ability: The ability of current player (also uses hidden).

    A parameter object (for the softmax calculation):
        b: A (K,) constant array.
        W: A (K, N_features) matrix, where in this case N_features=4 (from the
           total values in derived state).

    The simulator then for each batch produces N_sims by finding which team
    reaches 10 first. The scores are then saved to a result array. The
    distribution at each events determines if:
        A player scores 0 point.
        A player scores 1 point.
        A player scores 2 points.

    So we have total outcomes K=3.

    Finally note that each team consists of 3 players who cycle alternately,
    each with their own hidden feature, so N_hidden=6. It is assumed the home
    team start.
    """
    # Sort out where to save data.
    dir_path = os.path.dirname(os.path.abspath(__file__))

    # Set constants.
    N_batches = batches
    K = 3
    N_features = 4
    N_hidden = 6

    # Generate data.
    with Timer() as t:
        # Hidden features.
        hidden = 0.1*np.random.rand(N_batches, N_hidden)

        # Parameters
        b = 0.1*np.random.rand(K, 1).astype(np.float64)
        W = 0.1*np.random.rand(K, N_features).astype(np.float64)
        print("Time to generate data: %s." % t.elapsed)

    # Save binary data.
    with Timer() as t:
        hidden.tofile(dir_path + '/hidden.bin')
        b.tofile(dir_path + '/b.bin')
        W.tofile(dir_path + '/W.bin')
        np.array([N_batches, N_hidden, K, N_features], dtype=np.int) \
            .tofile(dir_path + '/meta.bin')
        print("Time to save binary data: %s." % t.elapsed)


if __name__ == "__main__":
    generate()

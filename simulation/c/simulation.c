// A program for reading csv data and writing to a new file.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <cblas.h>
#include <math.h>
#include <gsl/gsl_rng.h>
#include <omp.h>

struct matrix {
    double *data;
    int N;  // Total entries in matrix.
    int N_row;
    int N_col;
};

struct State {
    int home_score;
    int away_score;
    int total_events;
};

struct Derived_State {
    double score;
    double is_home;
    double is_winning;
    double player_ability;
};

enum RNG_Type {
    GSL,
    LCG
};

struct RNG {
    // A random number generator structure. It can be any of a GSL random
    // number generator, to a linear congruential generator (LCG).

    // We use the following types,
    //     0: GSL uniform random generator.
    //     1: LCG generator.
    enum RNG_Type type;

    // Parameters associated with LCG.
    int seed;
    int a;
    int c;
    int m;

    // Parameters associated with GSL.
    gsl_rng *r;   // GSL random number generator
};

int set_GSL_RNG(struct RNG *rng, int thread_number) {
    // Set parameters of the rng object for the GSL generator.

    //rng->r = gsl_rng_alloc(gsl_rng_taus);  // The Tausworthe generator
    rng->r = gsl_rng_alloc(gsl_rng_mt19937);  // The Mersenne Twister
    //rng->r = gsl_rng_alloc(gsl_rng_ranlxs0);  // Luxury random numbers level 2 (accurate but slow)
    //rng->r = gsl_rng_alloc(gsl_rng_ranlxs2);  // Luxury random numbers level 2 (accurate but slow)

    gsl_rng_set(rng->r, time(NULL) + rng->seed + thread_number*101);  // Seed the RNG
    srand(time(NULL) + rng->seed);

    return 0;
}

int free_GSL_RNG(struct RNG *rng) {
    // Free parameters of the rng object for the GSL generator.
    gsl_rng_free(rng->r);

    return 0;
}

double sample_RNG(struct RNG *rng) {
    // Samples a uniform random value in [0,1] using our random number
    // generator.
    double RU;
    if (rng->type == GSL) {
        // Use the GSL generator.
        RU = (double) gsl_rng_uniform(rng->r);
    } else {
        // Use the LCG.
        // Move the seed forward before sampling.
        rng->seed = (rng->seed * rng->a + rng->c) % rng->m;
        RU = (double) rng->seed / (double) (rng->m - 1);
    }

    // RU = ((float_type) rand())/ ((float_type) RAND_MAX);


    return RU;
}

int main(int argc, char *argv[]) {
    // Read the data.
    FILE *fp;
    FILE *fp_home;
    FILE *fp_away;
    int i, j;
    struct matrix b;
    struct matrix W;
    struct matrix hidden;
    clock_t now;
    int use_lcg;
    int save;
    int N_simulations;
    int thread_number;


    // Read options from command line.
    i = 1;
    N_simulations = 10;
    save = 0;
    use_lcg = 0;
    while (i<argc) {
        if (!strcmp(argv[i], "--simulations")) {
            N_simulations = atoi(argv[i+1]);
            i++;
        } else if (!strcmp(argv[i], "--use-lcg")) {
            use_lcg = 1;
        } else if (!strcmp(argv[i], "--save")) {
            if (omp_get_max_threads() != 1) {
                printf("Warning: Saving when using more than 1 thread. The results will not be comparable with Python.\n");
            }
            save = 1;
        }
        i++;
    }

    char *fname_load_b = "../data/b.bin";
    char *fname_load_W = "../data/W.bin";
    char *fname_load_hidden = "../data/hidden.bin";
    char *fname_load_meta = "../data/meta.bin";
    char *fname_save_home = "../data/home_c.csv";
    char *fname_save_away = "../data/away_c.csv";

    // First perform the easy load of the binary data.
    now = clock();

    long M_bin_size[4];
    fp = fopen(fname_load_meta, "rb");
    if (!fp) {
        exit(EXIT_FAILURE);
    }
    fread(M_bin_size, sizeof(long), 4, fp);
    fclose(fp);

    int N_batches = M_bin_size[0];
    int N_hidden = M_bin_size[1];
    int K = M_bin_size[2];
    int N_features = M_bin_size[3];

    b.N_row = K;
    b.N_col = 1;
    b.N = b.N_row * b.N_col;

    W.N_row = K;
    W.N_col = N_features;
    W.N = W.N_row * W.N_col;

    hidden.N_row = N_batches;
    hidden.N_col = N_hidden;
    hidden.N = hidden.N_row * hidden.N_col;

    // Load b.
    fp = fopen(fname_load_b, "rb");
    if (!fp) {
        exit(EXIT_FAILURE);
    }
    b.data = malloc(b.N * sizeof(double));
    fread(b.data, sizeof(double), b.N, fp);
    fclose(fp);

    // Load W.
    fp = fopen(fname_load_W, "rb");
    if (!fp) {
        exit(EXIT_FAILURE);
    }
    W.data = malloc(W.N * sizeof(double));
    fread(W.data, sizeof(double), W.N, fp);
    fclose(fp);

    // Load hidden.
    fp = fopen(fname_load_hidden, "rb");
    if (!fp) {
        exit(EXIT_FAILURE);
    }
    hidden.data = malloc(hidden.N * sizeof(double));
    fread(hidden.data, sizeof(double), hidden.N, fp);
    fclose(fp);

    printf("LOAD_ALLOCATE_BIN: %.10f.\n", (double) (clock() - now)/CLOCKS_PER_SEC);

    // Create object for storing results in.
    int *result = malloc(N_batches * N_simulations * 2 * sizeof(int));

    // Perform the simulation.
    double now_double = omp_get_wtime();

#pragma omp parallel private(i, j)
    {
        // Set the GSL generator if nead be.
        struct RNG rng;

        // Set RNG parameters.
        rng.a = 106;
        rng.c = 1283;
        rng.m = 6075;
        rng.seed = 42;

        if (use_lcg) {
            rng.type = LCG;
        } else {
            rng.type = GSL;
        }

#ifdef _OPENMP
        thread_number = omp_get_thread_num();
#else
        thread_number = 0;
#endif

        if (rng.type == GSL) {
            set_GSL_RNG(&rng, thread_number);
        }

        // Create object for storing distribution and feature vector.
        double *dist = malloc(K * sizeof(double));
        double *x = malloc(N_features * sizeof(double));

        struct State state;
        struct Derived_State derived_state;
        int simulation;
        int batch;
        int away;
        int home;
        int player;
        double dist_sum;
        double u;
        double dist_cum;
        int outcome;

        #pragma omp for
        for (i=0; i<N_batches * N_simulations; i++) {
            // Inner loop over batches for most diverse input if cut short.
            simulation = i / N_batches;
            batch = i % N_batches;

            // Set state.
            state.home_score = 0;
            state.away_score = 0;
            state.total_events = 0;

            while ((state.home_score < 10) & (state.away_score < 10)) {
                // Get derived state.
                away = state.total_events % 2;
                home = 1 - away;

                player = state.total_events % 6;

                derived_state.score = (double) (state.home_score * home +
                                                state.away_score * away);

                derived_state.is_home = (double) 1 - away;

                derived_state.is_winning = (double)
                    (state.home_score > state.away_score) * home + \
                    (state.home_score < state.away_score) * away;

                derived_state.player_ability = 
                    hidden.data[batch*hidden.N_col + player];

                // Get the distributions.
                x[0] = derived_state.score;
                x[1] = derived_state.is_home;
                x[2] = derived_state.is_winning;
                x[3] = derived_state.player_ability;

                for (j=0; j<K; j++) dist[j] = b.data[j];

                cblas_dgemv(CblasRowMajor,
                            CblasNoTrans,
                            W.N_row,
                            W.N_col,
                            1.0,
                            W.data,
                            W.N_col,
                            x,
                            1,
                            1.0,
                            dist,
                            1);

                dist_sum = 0;
                for (j=0; j<K; j++) {
                    dist[j] = exp(dist[j]);
                    dist_sum += dist[j];
                }

                // Simulate event from distribution.
                u = dist_sum * sample_RNG(&rng);
                dist_cum = 0;
                outcome = K - 1;
                for (j=0; j<K-1; j++) {
                    dist_cum += dist[j];
                    if (u < dist_cum) {
                        outcome = j;
                    }
                }

                // Apply the outcome to the state.
                state.total_events += 1;

                state.home_score += home * outcome;
                state.away_score += away * outcome;
            }

            // Match has ended so fill in scores into result array.
            result[batch*N_simulations*2 + simulation*2 + 0] = state.home_score;
            result[batch*N_simulations*2 + simulation*2 + 1] = state.away_score;
        }

        free(x);
        free(dist);

        if (rng.type == GSL) {
            free_GSL_RNG(&rng);
        }
    }
    printf("SIMULATION: %.10f.\n", omp_get_wtime() - now_double);


    // Save the data to a new csv.
    if (save) {
        now = clock();
        fp_home = fopen(fname_save_home, "wb");
        if (!fp_home) {
            exit(EXIT_FAILURE);
        }

        fp_away = fopen(fname_save_away, "wb");
        if (!fp_away) {
            exit(EXIT_FAILURE);
        }

        for (i=0; i<N_batches; i++) {
            // Have to do this weird way to get the commas correct.
            fprintf(fp_home, "%d", result[i*N_simulations*2 + 0*2 + 0]);
            fprintf(fp_away, "%d", result[i*N_simulations*2 + 0*2 + 1]);

            for (j=1; j<N_simulations; j++) {
                fprintf(fp_home, ",%d", result[i*N_simulations*2 + j*2 + 0]);
                fprintf(fp_away, ",%d", result[i*N_simulations*2 + j*2 + 1]);
            }
            fprintf(fp_home, "\n");
            fprintf(fp_away, "\n");
        }


        fclose(fp_home);
        fclose(fp_away);

        printf("SAVE_CSV: %.10f.\n", (double) (clock() - now)/CLOCKS_PER_SEC);
    }

    free(b.data);
    free(W.data);
    free(hidden.data);

    exit(EXIT_SUCCESS);
}

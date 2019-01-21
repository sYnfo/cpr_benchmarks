// A program for reading csv data and writing to a new file.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <gsl/gsl_rng.h>
#include <omp.h>

struct matrix {
    double *data;
    int N;  // Total entries in matrix.
    int N_row;
    int N_col;
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
    int i, j;
    clock_t now;
    int use_lcg;
    int save;
    struct matrix M_csv;
    int thread_number;


    // Read options from command line.
    M_csv.N_row = 10000;
    M_csv.N_col = 5;

    i = 1;
    save = 0;
    use_lcg = 0;
    while (i<argc) {
        if (!strcmp(argv[i], "--rows")) {
            M_csv.N_row = atoi(argv[i+1]);
            i++;
        } else if (!strcmp(argv[i], "--cols")) {
            M_csv.N_col = atoi(argv[i+1]);
            i++;
        } else if (!strcmp(argv[i], "--use-lcg")) {
            use_lcg = 1;
        } else if (!strcmp(argv[i], "--save")) {
            save = 1;
        }
        i++;
    }

    M_csv.N = M_csv.N_row * M_csv.N_col;


    char *fname_save = "../data/data_c.csv";

    // Perform the sampling.
    M_csv.data = malloc(M_csv.N * sizeof(double));

    double now_double = omp_get_wtime();

#pragma omp parallel private(i)
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

        #pragma omp for
        for (i=0; i<M_csv.N; i++) {
            M_csv.data[i] = sample_RNG(&rng);
        }

        if (rng.type == GSL) {
            free_GSL_RNG(&rng);
        }
    }
    printf("SAMPLE: %.10f.\n", omp_get_wtime() - now_double);


    // Save the data to a new csv.
    if (save) {
        now = clock();
        fp = fopen(fname_save, "wb");
        if (!fp) {
            exit(EXIT_FAILURE);
        }

        for (i=0; i<M_csv.N_row; i++) {
            // Have to do this weird way to get the commas correct.
            fprintf(fp, "%.10f", M_csv.data[i*M_csv.N_col]);

            for (j=1; j<M_csv.N_col; j++) {
                fprintf(fp, ",%.10f", M_csv.data[i*M_csv.N_col + j]);
            }
            fprintf(fp, "\n");
        }


        fclose(fp);

        printf("SAVE_CSV: %.10f.\n", (double) (clock() - now)/CLOCKS_PER_SEC);
    }

    free(M_csv.data);

    exit(EXIT_SUCCESS);
}

// A program for reading csv data and writing to a new file.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <omp.h>

struct matrix {
    double *data;
    int N;  // Total entries in matrix.
    int N_row;
    int N_col;
};

int main(int argc, char *argv[]) {
    // Read the data.
    FILE *fp;
    int i, j;
    struct matrix M_bin;
    clock_t now;
    int save;

    // Get whether to save files.
    if (argc == 1) {
        save = 0;
    } else if (!strcmp(argv[1], "--save")) {
        save = 1;
    } else {
        save = 0;
    }

    char *fname_load_bin;
    char *fname_load_meta_bin;
    char *fname_save;

    fname_load_bin = "../data/data.bin";
    fname_load_meta_bin = "../data/data_meta.bin";
    fname_save = "../data/data_c.csv";

    // First perform the easy load of the binary data.
    now = clock();

    long M_bin_size[2];
    fp = fopen(fname_load_meta_bin, "rb");
    if (!fp) {
        exit(EXIT_FAILURE);
    }
    fread(M_bin_size, sizeof(long), 2, fp);
    fclose(fp);

    M_bin.N_row = M_bin_size[0];
    M_bin.N_col = M_bin_size[1];
    M_bin.N = M_bin.N_row * M_bin.N_col;

    fp = fopen(fname_load_bin, "rb");
    if (!fp) {
        exit(EXIT_FAILURE);
    }
    M_bin.data = malloc(M_bin.N * sizeof(double));
    fread(M_bin.data, sizeof(double), M_bin.N, fp);
    fclose(fp);

    printf("LOAD_ALLOCATE_BIN: %.10f.\n", (double) (clock() - now)/CLOCKS_PER_SEC);


    // Exponentiate the data in place.
    double now_double = omp_get_wtime();
#pragma omp parallel for
    for (i=0; i<M_bin.N; i++) {
        M_bin.data[i] = exp(M_bin.data[i]);
    }

    printf("EXP: %.10f.\n", omp_get_wtime() - now_double);

    // Save the data to a new csv.
    if (save) {
        now = clock();
        fp = fopen(fname_save, "wb");
        if (!fp) {
            exit(EXIT_FAILURE);
        }

        for (i=0; i<M_bin.N_row; i++) {
            // Have to do this weird way to get the commas correct.
            fprintf(fp, "%.10f", M_bin.data[i*M_bin.N_col]);

            for (j=1; j<M_bin.N_col; j++) {
                fprintf(fp, ",%.10f", M_bin.data[i*M_bin.N_col + j]);
            }
            fprintf(fp, "\n");
        }


        fclose(fp);

        printf("SAVE_CSV: %.10f.\n", (double) (clock() - now)/CLOCKS_PER_SEC);
    }

    free(M_bin.data);

    exit(EXIT_SUCCESS);
}

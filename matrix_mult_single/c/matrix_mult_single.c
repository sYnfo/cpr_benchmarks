// A program for reading csv data and writing to a new file.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <cblas.h>
#include <omp.h>

struct matrix {
    double *data;
    int N;  // Total entries in matrix.
    int N_row;
    int N_col;
    int pointer;
};

int main(int argc, char *argv[]) {
    // Read the data.
    FILE *fp;
    int i, j;
    struct matrix A;
    struct matrix B;
    struct matrix C;
    clock_t now;
    int save;

    // Get the file paths by considering whether to use RAM.
    if (argc == 1) {
        save = 0;
    } else if (!strcmp(argv[1], "--save")) {
        save = 1;
    } else {
        save = 0;
    }

    char *fname_load_A = "../data/A.bin";
    char *fname_load_B = "../data/B.bin";
    char *fname_load_meta = "../data/meta.bin";
    char *fname_save = "../data/C_c.csv";

    // First perform the easy load of the binary data.
    now = clock();

    long M_bin_size[3];
    fp = fopen(fname_load_meta, "rb");
    if (!fp) {
        exit(EXIT_FAILURE);
    }
    fread(M_bin_size, sizeof(long), 3, fp);
    fclose(fp);

    A.N_row = M_bin_size[0];
    A.N_col = M_bin_size[1];
    A.N = A.N_row * A.N_col;

    B.N_row = M_bin_size[1];
    B.N_col = M_bin_size[2];
    B.N = B.N_row * B.N_col;

    C.N_row = M_bin_size[0];
    C.N_col = M_bin_size[2];
    C.N = C.N_row * C.N_col;

    // Load A.
    fp = fopen(fname_load_A, "rb");
    if (!fp) {
        exit(EXIT_FAILURE);
    }
    A.data = malloc(A.N * sizeof(double));
    fread(A.data, sizeof(double), A.N, fp);
    fclose(fp);

    // Load B.
    fp = fopen(fname_load_B, "rb");
    if (!fp) {
        exit(EXIT_FAILURE);
    }
    B.data = malloc(B.N * sizeof(double));
    fread(B.data, sizeof(double), B.N, fp);
    fclose(fp);

    printf("LOAD_ALLOCATE_BIN: %.10f.\n", (double) (clock() - now)/CLOCKS_PER_SEC);

    // Perform the matrix multiplication.
    double now_double = omp_get_wtime();

    C.data = malloc(C.N * sizeof(double));

    cblas_dgemm(CblasRowMajor,
                CblasNoTrans,
                CblasNoTrans,
                A.N_row,
                B.N_col,
                A.N_col,
                1.0,
                A.data,
                A.N_col,
                B.data,
                B.N_col,
                0.0,
                C.data,
                C.N_col);

    printf("MATRIX_MULT_SINGLE: %.10f.\n", (double) (omp_get_wtime() - now_double));

    // Save the data to a new csv.
    if (save) {
        now = clock();
        fp = fopen(fname_save, "wb");
        if (!fp) {
            exit(EXIT_FAILURE);
        }

        for (i=0; i<C.N_row; i++) {
            // Have to do this weird way to get the commas correct.
            fprintf(fp, "%.10f", C.data[i*C.N_col]);

            for (j=1; j<C.N_col; j++) {
                fprintf(fp, ",%.10f", C.data[i*C.N_col + j]);
            }
            fprintf(fp, "\n");
        }


        fclose(fp);

        printf("SAVE_CSV: %.10f.\n", (double) (clock() - now)/CLOCKS_PER_SEC);
    }

    free(A.data);
    free(B.data);
    free(C.data);

    exit(EXIT_SUCCESS);
}

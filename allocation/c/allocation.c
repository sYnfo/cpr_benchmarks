// A program for reading csv data and writing to a new file.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <csv.h>

struct matrix {
    double *data;
    int N;  // Total entries in matrix.
    int N_row;
    int N_col;
    int pointer;
};

// Define the callback functions for after each field and row is read.
void field_count(void *s, size_t len, void *data) {
    ((struct matrix *) data)->N++;
}

void row_count(int c, void *data) {
    ((struct matrix *) data)->N_row++;
}

void field_read(void *s, size_t len, void *data) {
    // I THINK I CANNOT USE LEN BUT INSTEAD PUT THE MATRIX IN A STRUCTURE!
    struct matrix *M = data;
    M->data[M->pointer] = atof((char *) s);
    M->pointer++;
}

int main(int argc, char *argv[]) {
    // Read the data.
    FILE *fp;
    struct csv_parser p;
    char buf[1024];
    size_t bytes_read;
    int i, j;
    struct matrix M_csv;
    struct matrix M_bin;
    clock_t now;
    int use_ram;

    M_csv.pointer = 0;
    M_csv.N = 0;
    M_csv.N_row = 0;

    // Get the file paths by considering whether to use RAM.
    if (argc == 1) {
        use_ram = 0;
    } else if (!strcmp(argv[1], "--use-ram")) {
        use_ram = 1;
    } else {
        use_ram = 0;
    }

    char *fname_load_csv;
    char *fname_load_bin;
    char *fname_load_meta_bin;
    char *fname_save;

    if (use_ram) {
        //printf("Using RAM.\n");
        fname_load_csv = "/dev/shm/allocation/data/data.csv";
        fname_load_bin = "/dev/shm/allocation/data/data.bin";
        fname_load_meta_bin = "/dev/shm/allocation/data/data_meta.bin";
        fname_save = "/dev/shm/allocation/data/data_c.csv";
    } else {
        //printf("Not Using RAM.\n");
        fname_load_csv = "../data/data.csv";
        fname_load_bin = "../data/data.bin";
        fname_load_meta_bin = "../data/data_meta.bin";
        fname_save = "../data/data_c.csv";
    }

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

    now = clock();

    if (csv_init(&p, 0) != 0) {
        exit(EXIT_FAILURE);
    }

    fp = fopen(fname_load_csv, "rb");
    if (!fp) {
        exit(EXIT_FAILURE);
    }

    // First pass through the data reads the total rows and columns.
    while ((bytes_read=fread(buf, 1, 1024, fp)) > 0) {
        if (csv_parse(&p, buf, bytes_read, field_count, row_count, &M_csv) != bytes_read) {
            fprintf(stderr, "Error while parsing file: %s\n",
                    csv_strerror(csv_error(&p)));
            exit(EXIT_FAILURE);
        }
    }

    csv_fini(&p, field_count, row_count, &M_csv);
    M_csv.N_col = M_csv.N/M_csv.N_row;

    // Second pass through the data reads the csv into memory.
    fseek(fp, 0, SEEK_SET);
    M_csv.data = malloc(M_csv.N * sizeof(double));
    while ((bytes_read=fread(buf, 1, 1024, fp)) > 0) {
        if (csv_parse(&p, buf, bytes_read, field_read, NULL, &M_csv) != bytes_read) {
            fprintf(stderr, "Error while parsing file: %s\n",
                    csv_strerror(csv_error(&p)));
            exit(EXIT_FAILURE);
        }
    }

    csv_fini(&p, field_read, NULL, &M_csv);

    fclose(fp);
    csv_free(&p);

    printf("LOAD_ALLOCATE_CSV: %.10f.\n", (double) (clock() - now)/CLOCKS_PER_SEC);

    // Check the values are the same.
    if (M_bin.N_row != M_csv.N_row) exit(EXIT_FAILURE);
    if (M_bin.N_col != M_csv.N_col) exit(EXIT_FAILURE);
    for (i=0; i<M_csv.N; i++) {
        if ((M_bin.data[i] - M_csv.data[i]) * (M_bin.data[i] - M_csv.data[i]) > 1e-6) exit(EXIT_FAILURE);
    }

    // Save the data to a new csv.
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

    free(M_csv.data);
    free(M_bin.data);

    exit(EXIT_SUCCESS);
}

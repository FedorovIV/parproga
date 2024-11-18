#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define ISIZE 100
#define JSIZE 100

int main(int argc, char **argv) {
    double a[ISIZE][JSIZE];
    double local_a[ISIZE][JSIZE]; // Местный массив
    int i, j;
    FILE *ff;

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Подготовительная часть – заполнение некими данными
    for (i = 0; i < ISIZE; i++) {
        for (j = 0; j < JSIZE; j++) {
            a[i][j] = 10 * i + j;
        }
    }

    // Разделим работу между процессами
    int rows_per_process = ISIZE / size;
    int start_row = rank * rows_per_process;
    int end_row = (rank + 1) * rows_per_process;
    if (rank == size - 1) {
        end_row = ISIZE; // Обрабатываем оставшиеся строки на последнем процессе
    }

    // Рассчитываем часть массива
    for (i = start_row; i < end_row; i++) {
        for (j = 0; j < JSIZE; j++) {
            local_a[i][j] = sin(2 * a[i][j]);
        }
    }

    // Собираем результаты на процессе 0
    MPI_Gather(local_a[start_row], (end_row - start_row) * JSIZE, MPI_DOUBLE, a, (end_row - start_row) * JSIZE, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Записываем результат в файл только процессом 0
    if (rank == 0) {
        ff = fopen("result.txt", "w");
        for (i = 0; i < ISIZE; i++) {
            for (j = 0; j < JSIZE; j++) {
                fprintf(ff, "%f ", a[i][j]);
            }
            fprintf(ff, "\n");
        }
        fclose(ff);
    }

    MPI_Finalize();
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <time.h>

#define ISIZE 500
#define JSIZE 500

int main(int argc, char **argv)
{
    double a[ISIZE][JSIZE];
    double local_a[ISIZE][JSIZE]; // Местный массив
    int i, j;
    FILE *ff;

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Подготовительная часть – заполнение некими данными
    for (i = 0; i < ISIZE; i++)
    {
        for (j = 0; j < JSIZE; j++)
        {
            a[i][j] = 10 * i + j;
        }
    }

    // Разделим работу между процессами
    int rows_per_process = ISIZE / size;
    int start_row = rank * rows_per_process;
    int end_row = (rank + 1) * rows_per_process;
    if (rank == size - 1)
    {
        end_row = ISIZE; // Обрабатываем оставшиеся строки на последнем процессе
    }

    // printf("Process %d sending data to process start row %d end_row %d\n", rank, start_row, end_row);

    clock_t start; // Переместил объявление сюда

    if (rank == 0)
    {
        // Измерение времени начала работы цикла
        start = clock();
    }
    // Рассчитываем часть массива
    for (i = start_row; i < end_row; i++)
    {
        for (j = 0; j < JSIZE; j++)
        {
            local_a[i][j] = sin(2 * a[i][j]);
        }
    }

    // Собираем результаты на процессе 0
    if (rank == 0)
    {
        for (i = start_row; i < end_row; i++){
            for (j = 0; j < JSIZE; j++){
                a[i][j] = local_a[i][j];
            }
        }

        // printf("second elems in local_a of 0 proc %f \n", local_a[0][1]);
        for (i = 1; i < size; i++)
        {
            int rows = ((i == size - 1) && (ISIZE % size != 0)) ? (ISIZE - rows_per_process*i) : rows_per_process;
            // printf("I am process 0, receiving data from process %d , with %d rows\n", i, rows);
            MPI_Recv(&a[i*rows_per_process], JSIZE * rows , MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
    else
    {
        MPI_Send(local_a[start_row], (end_row - start_row) * JSIZE, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    if (rank == 0)
    {
        // Измерение времени окончания работы цикла
        clock_t end = clock();

        // Вычисление времени выполнения
        double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

        // Вывод времени выполнения?
        printf("Time taken for computation: %f seconds\n", time_taken);
    }

    // Записываем результат в файл только процессом 0
    if (rank == 0)
    {
        ff = fopen("resultMPI.txt", "w");
        for (i = 0; i < ISIZE; i++)
        {
            for (j = 0; j < JSIZE; j++)
            {
                fprintf(ff, "%f ", a[i][j]);
            }
            fprintf(ff, "\n");
        }
        fclose(ff);
    }

    MPI_Finalize();
    return 0;
}
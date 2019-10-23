#include <mpi.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <vector>

using namespace std;

const size_t ROOT = 0;

int swap(int *array, int i) {
    if (array[i] > array[i+1]) {
        int tmp = array[i];
        array[i] = array[i+1];
        array[i+1] = tmp;
    }
    return 0;
}

int main(int argc, char **argv) {
    int global_size = atoi(argv[1]);
    int number_of_processors, rank_of_processor, actual_global_size;
    int *global_array = (int *)malloc(sizeof(int)*global_size);
    int new_sorted_global_array[global_size];
    int remainder, local_size_max, local_size;
    int send_right, recv_right, send_left, recv_left;
    double start_time, end_time;

    /* generate numbers to create array data of input size */

    srand(2019);  //set the seed
    for (int i = 0; i < global_size; i++) {
        global_array[i] = ((int)rand()) % 1000;  //generate random numbers smaller than 1000
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &number_of_processors);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_of_processor);

    start_time = MPI_Wtime();

    if (number_of_processors <= 1) {
        printf("\nError! Number of processors must be larger than 1\n" );
        MPI_Finalize();
        exit(0);
    }

    /* make each processor have even number of numbers
     * the last processor has the remaining numbers */

    remainder = global_size % number_of_processors;
    local_size = global_size / number_of_processors;
    local_size_max = local_size;
    actual_global_size = global_size;
    if (local_size % 2 == 1) {
        local_size += 1;  // case 1 array: |odd+1|odd+1|...|XXX+n*1000|
        local_size_max = local_size;
        actual_global_size = local_size * number_of_processors;
    }
    else if (rank_of_processor == number_of_processors-1) {
        local_size_max = local_size + remainder;  // case 2 array: |even|even|...|even+remainder|
    }
    int *sorted_global_array = (int *)malloc(sizeof(int)*actual_global_size);

    /* scatter the global array data to the local array in each process */

    int *local_array = (rank_of_processor == number_of_processors-1)
            ? (int *)malloc(sizeof(int)*local_size_max) : (int *)malloc(sizeof(int)*local_size);
    for (int i = 0; i < local_size_max; i++) {
        if (local_size*rank_of_processor+i < global_size) {
            local_array[i] = global_array[local_size*rank_of_processor+i];
        }
        else{
            local_array[i] = 1000;  // 1000 is the maximum number
        }
    }

    if (rank_of_processor == ROOT) {
        printf("\nName: Ran Hu\n");
        printf("Student ID: 116010078\n");
        printf("Assignment 1: Odd-Even Transposition Sort -- MPI Implementation\n\n");
        printf("Sorting %d random numbers using %d processors\n\n", global_size, number_of_processors);
        printf("Orignal array: ");
        for (int i = 0; i < global_size; i++) {
            printf("%d ", global_array[i]);
        }
    }

    /* sorting process */

    for (int iteration_number = 0; iteration_number < global_size; iteration_number++) {

        /* odd comparison */

        if (iteration_number % 2 == 1) {
            for (int local_array_index = 0; local_array_index < local_size_max/2; local_array_index++) {
                swap(local_array, 2*local_array_index);
            }
        }

        /* even comparison & boundary comparison*/

        else {
            if (rank_of_processor == number_of_processors-1) {
                send_left = local_array[0];
                MPI_Send(&send_left, 1, MPI_INT, rank_of_processor-1, 0, MPI_COMM_WORLD);
            } else if (rank_of_processor != ROOT) {
                MPI_Recv(&recv_right, 1, MPI_INT, rank_of_processor+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                send_left = local_array[0];
                MPI_Send(&send_left, 1, MPI_INT, rank_of_processor-1, 0, MPI_COMM_WORLD);
            } else if (rank_of_processor == ROOT && number_of_processors != 1) {
                MPI_Recv(&recv_right, 1, MPI_INT, rank_of_processor+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            for (int local_array_index = 0; local_array_index < local_size_max/2; local_array_index++) {
                if (local_size_max % 2 != 0) {
                    swap(local_array, 2*local_array_index+1);
                } 
                else {
                    if (local_array_index < local_size_max/2-1) {
                        swap(local_array, 2*local_array_index+1);
                    }
                    else if (rank_of_processor != number_of_processors-1) {
                         if (local_array[local_size-1] > recv_right) {
                             send_right = local_array[local_size-1];  //swap the boundary
                             local_array[local_size-1] = recv_right;
                         }
                         else send_right = recv_right;
                    }
                }
            }
            if (rank_of_processor == ROOT) {
                MPI_Send(&send_right, 1, MPI_INT, rank_of_processor+1, 0, MPI_COMM_WORLD);
            } else if (rank_of_processor != number_of_processors-1) {
                MPI_Recv(&recv_left, 1, MPI_INT, rank_of_processor-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                local_array[0] = recv_left;
                MPI_Send(&send_right, 1, MPI_INT, rank_of_processor+1, 0, MPI_COMM_WORLD);
            } else if (rank_of_processor == number_of_processors-1) {
                MPI_Recv(&recv_left, 1, MPI_INT, rank_of_processor-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                local_array[0] = recv_left;
            }
        }
    }

    /* gather all the data */

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Gather(local_array, local_size, MPI_INT, sorted_global_array, local_size, MPI_INT, 0, MPI_COMM_WORLD);

    int *send_remainder = (int *)malloc(sizeof(int)*remainder);
    int *recv_remainder = (int *)malloc(sizeof(int)*remainder);
    if (((global_size/number_of_processors)%2 == 0) && (remainder > 0)) {
        if (rank_of_processor == number_of_processors-1) {
            for (int i = 0; i < remainder; i++) {
                send_remainder[i] = local_array[local_size + i];
            }
            MPI_Send(send_remainder, remainder, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
        else if (rank_of_processor == ROOT) {
            MPI_Recv(recv_remainder, remainder, MPI_INT, number_of_processors-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < remainder; i++) {
                sorted_global_array[global_size-remainder+i] = recv_remainder[i];
            }
        }
    }
    for (int i = 0; i < global_size; i++) {
        new_sorted_global_array[i] = sorted_global_array[i];
    }

    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();

    if (rank_of_processor == ROOT) {
        printf("\n\nSorted array: ");
        for (int i = 0; i < global_size; i++) {
            printf("%d ", new_sorted_global_array[i]);
        }
        printf("\n\nRunning time: %f\n\n", end_time - start_time);
    }

    MPI_Finalize();
    return 0;
}
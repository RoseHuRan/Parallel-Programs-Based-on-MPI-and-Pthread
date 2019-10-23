#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <chrono> 
#include <vector>

using namespace std;
using namespace chrono;

const size_t ROOT = 0;

int swap(int *array, int i) {
    int tmp = array[i];
    array[i] = array[i+1];
    array[i+1] = tmp;
    return 0;
}

int main(int argc, char **argv) {
    string argvi(argv[1]);
    int global_size = atoi(argv[1]);
    int *global_array = (int *)malloc(sizeof(int)*global_size);
    int ODD_EVEN = 0;
    int FINISH = 0;

    /* generate random numbers to create array data of input size */

    srand(2019);  //set the seed
    for (int i = 0; i < global_size; i++) {
        global_array[i] = ((int)rand()) % 1000;  //generate random numbers smaller than 1000
    }

    printf("\nName: Ran Hu\n");
    printf("Student ID: 116010078\n");
    printf("Assignment 1: Odd-Even Transposition Sort -- Sequential Implementation\n\n");
    printf("Sorting %d random numbers\n\n", global_size);
    printf("Orignal array: ");
    for (int i = 0; i < global_size; i++) {
        printf("%d ", global_array[i]);
    }

    auto start_time = system_clock::now();
    
    /* sorting process */

    while (FINISH != 1){
        FINISH = 1;
        for (int i = ODD_EVEN; i < global_size-1; i += 2){
    	    if (global_array[i] > global_array[i+1]) {
    			FINISH = 0;
            	swap(global_array, i);
            }
        }
        ODD_EVEN = !ODD_EVEN;
    }

    auto end_time = system_clock::now();
    auto duration = duration_cast<microseconds>(end_time - start_time);

    printf("\n\nSorted array: ");
    for (int i = 0; i < global_size; i++) {
        printf("%d ", global_array[i]);
    }
    printf("\n\nRunning time: %f\n\n", double(duration.count()) * microseconds::period::num / microseconds::period::den);

    return 0;
}
/**
 * @file parallel_mergesort.cpp
 * @author lulunkaii
 * @brief Parallel Merge Sort implementation
**/

#include <iostream>
#include <omp.h>
#include <vector>

#include "seq_mergesort.h"
#include "parallel_mergesort.h"

int CUTOFF = 25;

void parallelMergeSort(std::vector<int>& array, int left, int right){
    if (right-left + 1 < CUTOFF) return sequentialMergeSort(array, left, right);

    if (left < right){
        int mid = (left + right) / 2;
        
        #pragma omp task default(none) shared(array) firstprivate(left, mid)
        parallelMergeSort(array, left, mid);
        
        #pragma omp task default(none) shared(array) firstprivate(mid, right)
        parallelMergeSort(array, mid + 1, right);

        #pragma omp taskwait
        merge(array, left, right, mid);
    }
}


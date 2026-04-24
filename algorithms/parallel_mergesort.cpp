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

void merge(std::vector<int>& array, int left, int right, int mid){
    int size_l = mid-left + 1;
    int size_r = right-mid;

    std::vector<int> L(size_l);
    std::vector<int> R(size_r);

    for (int i = 0; i < size_l; i++){
        L[i] = array[left + i];
    }
    for (int j = 0; j < size_r; j++){
        R[j] = array[mid + j + 1];
    }
    int i = 0;
    int j= 0 ;
    int k = left;

    while(i < size_l && j < size_r){
        if(L[i] <= R[j]){
            array[k] = L[i];
            i++;
        } else {
            array[k] = R[j];
            j++;
        } 
        k++;
    }
    while (i < size_l){
        array[k] = L[i];
        i++;
        k++;
    }
    while (j < size_r){
        array[k] = R[j];
        j++;
        k++;
    }
}

void parallelMergeSort(std::vector<int>& array, int left, int right){
    if (right-left + 1 < CUTOFF) return sequentialMergeSort(array, left, right);

    if (left < right){
        int mid = (left + right) / 2;
        
        #pragma omp task default(none) 
        parallelMergeSort(array, left, mid);
        
        #pragma omp task default(none)
        parallelMergeSort(array, mid + 1, right);

        #pragma omp taskwait
        merge(array, left, right, mid);
    }
}


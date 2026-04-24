/** *
 * @file seq_mergesort.cpp
 * @author lulunkaii
 * @brief Sequential Merge Sort implementation
**/

#include <iostream>
#include <vector>

#include "seq_mergesort.h"

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

void sequentialMergeSort(std::vector<int>& array, int left, int right){
    if (left < right){
        int mid = (left + right) / 2;
        sequentialMergeSort(array, left, mid);
        sequentialMergeSort(array, mid + 1, right);
        merge(array, left, right, mid);
    }
}


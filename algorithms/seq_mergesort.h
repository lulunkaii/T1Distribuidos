#include <vector>

#ifndef seq_mergesort
#define seq_mergesort

void merge(std::vector<int>& array, int left, int right, int mid);
void sequentialMergeSort(std::vector<int>& array, int left, int right);

#endif
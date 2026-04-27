/** *
 * @file parallel_kway.cpp
 * @author Jhoossmm (
 * @brief K-way Merge Sort parallel implementation
**/

#include <iostream>
#include <vector>

#include <queue>
#include <algorithm>
#include <utility>
#include <omp.h>

#include "parallel_kway.h"

struct ParallelHeapNode {
    int value;
    int partition_index;
    int element_index;
    bool operator>(const ParallelHeapNode& other) const { return value > other.value; }
};

// Se mantiene secuencial, pues el costo de paralelizar el merge no compensa el costo de crear tareas.
void parallelKWay_merge(std::vector<int>& arr, const std::vector<std::pair<int, int>>& partitions) {
    if (partitions.empty()) return;
    int total_size = partitions.back().second - partitions.front().first + 1;
    std::vector<int> temp(total_size);
    std::priority_queue<ParallelHeapNode, std::vector<ParallelHeapNode>, std::greater<ParallelHeapNode>> min_heap;

    for (size_t i = 0; i < partitions.size(); ++i) {
        if (partitions[i].first <= partitions[i].second) {
            min_heap.push({arr[partitions[i].first], static_cast<int>(i), partitions[i].first});
        }
    }

    int temp_idx = 0;
    while (!min_heap.empty()) {
        ParallelHeapNode min_node = min_heap.top();
        min_heap.pop();
        temp[temp_idx++] = min_node.value;

        int p_idx = min_node.partition_index;
        int next_elem_idx = min_node.element_index + 1;

        if (next_elem_idx <= partitions[p_idx].second) {
            min_heap.push({arr[next_elem_idx], p_idx, next_elem_idx});
        }
    }

    int start_idx = partitions.front().first;
    for (int i = 0; i < total_size; ++i) {
        arr[start_idx + i] = temp[i];
    }
}

void parallelKWay_recursive(std::vector<int>& arr, int left, int right, int k) {
    if (left >= right) return;

    int size = right - left + 1;
    int current_k = std::min(k, size);

    int chunk_size = size / current_k;
    int remainder = size % current_k;

    std::vector<std::pair<int, int>> partitions;
    int current_left = left;

    for (int i = 0; i < current_k; ++i) {
        int current_chunk = chunk_size + (i < remainder ? 1 : 0);
        int current_right = current_left + current_chunk - 1;
        partitions.push_back({current_left, current_right});

        // Para el manejo de la granularidad, solo se crearán tareas OpenMP si el costo de crearlas (overhead)vale la pena
        // Si el tamaño es menor al threshold, no creamos más tareas OpenMP y ejecutamos secuenciamente
        int threshold = 4096;
        #pragma omp task shared(arr) if(size >= threshold)
        parallelKWay_recursive(arr, current_left, current_right, k);

        current_left = current_right + 1;
    }

    #pragma omp taskwait

    parallelKWay_merge(arr, partitions);
}

void parallelKWay(std::vector<int>& array, int left, int right, int k){
    if (array.empty() || left >= right || k < 2) return;
    
   parallelKWay_recursive(array, left, right, k);
}
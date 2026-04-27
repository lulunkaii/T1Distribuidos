#include <iostream>
#include <vector>

#include <queue>
#include <algorithm>
#include <utility>

#include "seq_kway.h"

struct HeapNode {
    int value;
    int partition_index;
    int element_index;

    bool operator>(const HeapNode& other) const {
        return value > other.value;
    }
};

void KWay_merge(std::vector<int>& arr, const std::vector<std::pair<int, int>>& partitions) {
    if (partitions.empty()) return;

    int total_size = partitions.back().second - partitions.front().first + 1;
    std::vector<int> temp(total_size); // Para guardar datos ordenados, temporalmente

    std::priority_queue<HeapNode, std::vector<HeapNode>, std::greater<HeapNode>> min_heap;

    // Toma el primer elemento de cada partición y lo inserta en el heap
    for (size_t i = 0; i < partitions.size(); ++i) {
        if (partitions[i].first <= partitions[i].second) {
            min_heap.push({arr[partitions[i].first], static_cast<int>(i), partitions[i].first});
        }
    }

    int temp_idx = 0;
    while (!min_heap.empty()) {
        // Saca el número más pequeño del heap y se guarda en temp
        HeapNode min_node = min_heap.top();
        min_heap.pop();

        temp[temp_idx++] = min_node.value;

        int p_idx = min_node.partition_index;
        int next_elem_idx = min_node.element_index + 1;

        if (next_elem_idx <= partitions[p_idx].second) {
            min_heap.push({arr[next_elem_idx], p_idx, next_elem_idx});
        }
    }

    int start_idx = partitions.front().first;
    for (int i = 0; i < total_size; ++i) { //Copia de temp a arr
        arr[start_idx + i] = temp[i];
    }
}

void KWay_mergesort_recursive(std::vector<int>& arr, int left, int right, int k) {
    if (left >= right) return; 

    int size = right - left + 1;
    int current_k = std::min(k, size); // Para asegurarse que k no sea mayor que el tamaño del arreglo

    int chunk_size = size / current_k;
    int remainder = size % current_k; // Restos luego de la división

    std::vector<std::pair<int, int>> partitions;

    int current_left = left;

    for (int i = 0; i < current_k; ++i) {
        int current_chunk = chunk_size + (i < remainder ? 1 : 0); //Distribuye el resto entre los primeros subarreglos
        int current_right = current_left + current_chunk - 1;

        partitions.push_back({current_left, current_right});

        KWay_mergesort_recursive(arr, current_left, current_right, k);
        current_left = current_right + 1;
    }

    KWay_merge(arr, partitions);
}

void sequentialKWay(std::vector<int>& array, int left, int right, int k){
    if (array.empty() || left >= right || k < 2) return;
    KWay_mergesort_recursive(array, left, right, k);
}
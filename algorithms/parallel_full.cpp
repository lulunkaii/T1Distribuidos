/**
 * @file parallel_full.cpp
 * @brief Implementación paralela completa: K-way division + BRMS parallel merge.
**/

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <utility>
#include <omp.h>

#include "parallel_full.h"

static const int FULL_CUT = 4096;
static const int FULL_C_FACTOR = 4;

// Lógica de BRMS para el merge

void full_partition(const std::vector<int>& A, int aLow, int aHigh,
            const std::vector<int>& B, int bLow, int bHigh,
            std::vector<int>& ranks,
            int logN) {

    int sizeA = aHigh - aLow;
    int numPartitions = ranks.size() - 1;

    ranks[0] = 0;
    ranks[numPartitions] = sizeA;

    #pragma omp parallel for schedule(static)
    for (int i = 1; i < numPartitions; i++) {
        int pivot = B[bLow + i * logN];
        auto it = std::lower_bound(A.begin() + aLow, A.begin() + aHigh, pivot);
        ranks[i] = std::distance(A.begin() + aLow, it);
    }
}

void full_seqMerge(const std::vector<int>& A, int aLow, int aHigh,
            const std::vector<int>& B, int bLow, int bHigh,
            std::vector<int>& output, int outLow) {

    int i = aLow, j = bLow, k = outLow;

    while (i < aHigh && j < bHigh)
        output[k++] = (A[i] <= B[j]) ? A[i++] : B[j++];
    
    while (i < aHigh) output[k++] = A[i++];
    while (j < bHigh) output[k++] = B[j++];
}

void full_parMergeBRMS(const std::vector<int>& A, int aLow, int aHigh,
                    const std::vector<int>& B, int bLow, int bHigh,
                    std::vector<int>& output, int outLow) {

    int sizeA = aHigh - aLow;
    int sizeB = bHigh - bLow;
    int totalN = sizeA + sizeB;

    if (sizeA == 0) {
        std::copy(B.begin() + bLow, B.begin() + bHigh, output.begin() + outLow);
        return;
    }
    if (sizeB == 0) {
        std::copy(A.begin() + aLow, A.begin() + aHigh, output.begin() + outLow);
        return;
    }

    if (totalN <= FULL_CUT) {
        full_seqMerge(A, aLow, aHigh, B, bLow, bHigh, output, outLow);
        return;
    }

    int logN = std::max(16, (int)std::log2(totalN));
    int numTasks = (sizeB + logN - 1) / logN;

    std::vector<int> ranks(numTasks + 1);

    full_partition(A, aLow, aHigh, B, bLow, bHigh, ranks, logN);

    #pragma omp parallel for 
    for (int i = 0; i < numTasks; i++) {

        int aStart = aLow + ranks[i];
        int aEnd   = aLow + ranks[i + 1];

        int bStart = bLow + i * logN;
        int bEnd   = std::min(bLow + (i + 1) * logN, bHigh);

        int outStart = outLow + ranks[i] + (i * logN);

        int subSizeA = aEnd - aStart;

        if (subSizeA <= FULL_C_FACTOR * logN) {
            full_seqMerge(A, aStart, aEnd, B, bStart, bEnd, output, outStart);
        } else {
            full_parMergeBRMS(B, bStart, bEnd, A, aStart, aEnd, output, outStart);
        }
    }
}

// Reducción en Árbol usando BRMS para mergear los arreglos ordenados del k-way mergesort
void tree_merge(std::vector<int>& arr, std::vector<int>& tmp, 
                     const std::vector<std::pair<int,int>>& parts, 
                     int p_start, int p_end, bool to_arr) {
    // Se recibe el arreglo original, un arreglo temporal, los límites de las partes (subarreglos ordenados) y el rango de subarreglos a mergear (p_start, p_end)
    // La idea es construir un árbol de merges dividiendo las partes en grupos (tipo torneo de llaves)
    // Para poder luego fusionar los subarreglos de manera que paralelizar sea eficiente

    // Caso base, cuando se llega a una sola partición en el grupo
    if (p_start == p_end) {
        if (!to_arr) {
            // Como se quiere que la partición quede en el arreglo temporal, se definen los limites y se copia a tmp
            int l_idx = parts[p_start].first;
            int r_idx = parts[p_start].second;
            std::copy(arr.begin() + l_idx, arr.begin() + r_idx + 1, tmp.begin() + l_idx);
        }
        return;
    }
    
    int mid = p_start + (p_end - p_start) / 2; //Se parte el grupo de subarreglos a la mitad: [[][][][]] - > [[][]] [[][]]
    
    // Se crean tareas para procesar cada mitad del árbol 
    // Se indica que dejen el resultado en el arreglo temporal(!to_arr)
    #pragma omp task shared(arr, tmp, parts)
    tree_merge(arr, tmp, parts, p_start, mid, !to_arr); //Primera mitad desde p_start a mid
    
    #pragma omp task shared(arr, tmp, parts)
    tree_merge(arr, tmp, parts, mid + 1, p_end, !to_arr); //Segunda mitad desde mid+1 a p_end
    
    #pragma omp taskwait //Se espera hasta que se creen todos los grupos
    
    //Se recuperan los límites de los subarreglos a fusionar
    int l_start = parts[p_start].first;
    int l_end = parts[mid].second;
    int r_start = parts[mid+1].first;
    int r_end = parts[p_end].second;
    
    // Fusionamos con BRMS
    //BRMS usa límites superiores exclusivos y los del arbol son inclusivos, por eso se suma 1 a los límites superiores
    if (to_arr) {
        full_parMergeBRMS(tmp, l_start, l_end + 1, tmp, r_start, r_end + 1, arr, l_start);
    } else {
        full_parMergeBRMS(arr, l_start, l_end + 1, arr, r_start, r_end + 1, tmp, l_start);
    }
}

// Lógica de k-way mergesort modificada para usar BRMS en la fase de merge

void parallelFull_recursive(std::vector<int>& arr, std::vector<int>& tmp, int left, int right, int k) {
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

        #pragma omp task shared(arr, tmp) if(size >= FULL_CUT)
        parallelFull_recursive(arr, tmp, current_left, current_right, k);

        current_left = current_right + 1;
    }

    #pragma omp taskwait

    // Cauando los subarreglos esten ordenadas internamente se aplica el árbol de merges BRMS
    tree_merge(arr, tmp, partitions, 0, current_k - 1, true);
}

void parallelFull(std::vector<int>& array, int left, int right, int k) {
    if (array.empty() || left >= right || k < 2) return;
    
    std::vector<int> tmp(array.size()); // Se crea el buffer temporal una vez para evitar cache misses y overhead
    
    parallelFull_recursive(array, tmp, left, right, k);
}
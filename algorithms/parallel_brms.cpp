/**
 * @file parallel_brms.cpp
 * @author amonlop
 * @brief Implementación paralela de Binary and Rank Mergesort (BRMS).
 * El algoritmo descompone el merge de dos arreglos ordenados seleccionando
 * pivotes del segundo arreglo y encontrando su posición (rank) en el primero
 * mediante búsqueda binaria, permitiendo realizar merges independientes en paralelo.
**/

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <omp.h>

#include "seq_mergesort.h"
#include "parallel_brms.h"

static const int CUT = 4096;     //umbral para utilizar sequential merge
static const int C_FACTOR = 4;          // factor c * logn


// Calcula los rangos (ranks) de los pivotes de B dentro de A.
void partition(const std::vector<int>& A, int aLow, int aHigh,
            const std::vector<int>& B, int bLow, int bHigh,
            std::vector<int>& ranks,
            int logN) {

    int sizeA = aHigh - aLow;
    int numPartitions = ranks.size() - 1;

    ranks[0] = 0;
    ranks[numPartitions] = sizeA;

    //la operación se puede hacer tanto paralela como secuencial, y depende del N
    #pragma omp parallel for schedule(static)
    for (int i = 1; i < numPartitions; i++) {
        int pivot = B[bLow + i * logN];

        auto it = std::lower_bound(A.begin() + aLow,
                                A.begin() + aHigh,
                                pivot);

        ranks[i] = std::distance(A.begin() + aLow, it);
    }
}


//versión alternativa de merge (para evitar uso de memoria innecesaria del merge definido en seq_mergesort.cpp, y con esto reducir los cache misses)
void seqMerge(const std::vector<int>& A, int aLow, int aHigh,
            const std::vector<int>& B, int bLow, int bHigh,
            std::vector<int>& output, int outLow) {

    int i = aLow, j = bLow, k = outLow;

    while (i < aHigh && j < bHigh)
        output[k++] = (A[i] <= B[j]) ? A[i++] : B[j++];

    while (i < aHigh) output[k++] = A[i++];
    while (j < bHigh) output[k++] = B[j++];
}

// merge paralelo considerando particiones y ranks binarios
void parMergeBRMS(const std::vector<int>& A, int aLow, int aHigh,
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

    if (totalN <= CUT) {
        seqMerge(A, aLow, aHigh, B, bLow, bHigh, output, outLow);
        return;
    }

    int logN = std::max(16, (int)std::log2(totalN));
    int numTasks = (sizeB + logN - 1) / logN;

    std::vector<int> ranks(numTasks + 1);

    partition(A, aLow, aHigh, B, bLow, bHigh, ranks, logN);

    #pragma omp parallel for 
    for (int i = 0; i < numTasks; i++) {

        int aStart = aLow + ranks[i];
        int aEnd   = aLow + ranks[i + 1];

        int bStart = bLow + i * logN;
        int bEnd   = std::min(bLow + (i + 1) * logN, bHigh);

        int outStart = outLow + ranks[i] + (i * logN);

        int subSizeA = aEnd - aStart;

        if (subSizeA <= C_FACTOR * logN) {
            seqMerge(A, aStart, aEnd,
                    B, bStart, bEnd,
                    output, outStart);
        } else {
            // se intercambian A y B
            parMergeBRMS(B, bStart, bEnd,
                        A, aStart, aEnd,
                        output, outStart);
        }
    }

}


void parallelBRMS(std::vector<int>& array, int left, int right) {

    if (left >= right) return;

    //se utiliza mergesort secuencial si el tamaño es menor al umbral CUT
    if (right - left + 1 <= CUT) {
        sequentialMergeSort(array, left, right);
        return;
    }

    int mid = left + (right - left) / 2;

    #pragma omp task default(none) shared(array) firstprivate(left, mid)
    parallelBRMS(array, left, mid);

    #pragma omp task default(none) shared(array) firstprivate(mid, right)
    parallelBRMS(array, mid + 1, right);

    #pragma omp taskwait

    //espacio temporal para almacenar la mezcla de las dos mitades
    std::vector<int> output(right - left + 1);

    parMergeBRMS(array, left, mid + 1,
                array, mid + 1, right + 1,
                output, 0);

    //cpia de los datos ordenados de vuelta al arreglo original
    std::copy(output.begin(), output.end(), array.begin() + left);
}
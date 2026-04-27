#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <numeric>
#include <algorithm>
#include <random>
#include <omp.h>

#include "seq_mergesort.h"
#include "parallel_mergesort.h"
#include "seq_kway.h"
#include "parallel_kway.h"
#include "parallel_brms.h"

struct Experiment
{
    std::string algorithm;
    int n;
    int threads;
    double time_avg;
    double time_min;
    double time_max;
    double speedup;
    double efficiency;
};


inline void validateInput(int argc, char* argv[], std::string& output_file, std::string& algorithm,
                          int& runs, int& exp_lower, int& exp_upper, int& exp_step, int& p_lower, int& p_upper, int& k_value) {
    if (argc != 10){
        std::cout << "Uso: ./mergesort <output.csv> <algorithm> <runs> <exp_lower> <exp_upper> <exp_step> <p_lower> <p_upper> <k_value>" << std::endl;
        std::cout << "output.csv : archivo de salida con resultados" << std::endl;
        std::cout << "algorithm : seq_mergesort | par_mergesort | seq_kway | par_kway | par_brms" << std::endl;
        std::cout << "runs : repeticiones por experimento" << std::endl;
        std::cout << "exp_lower : exponente minimo para n (n = 2^exp_lower)" << std::endl;
        std::cout << "exp_upper : exponente maximo para n (n = 2^exp_upper)" << std::endl;
        std::cout << "exp_step : salto entre exponentes (multiplicativo, 2^step)" << std::endl;
        std::cout << "p_lower : exponente minimo de threads (p = 2^p_lower)" << std::endl;
        std::cout << "p_upper : exponente maximo de threads (p = 2^p_upper)" << std::endl;
        std::cout << "k_value : valor de k para k-way merge" << std::endl;
        std::cout << "\n Ejemplo: ./mergesort output/results.csv par_mergesort 5 20 26 2 0 3 8" << std::endl;
        std::cout << "\t -> n en {2^20, 2^22, 2^24, 2^26} y p en {1,2,4,8}" << std::endl;
        std::exit(EXIT_FAILURE);
    } try {
        output_file = argv[1];
        algorithm   = argv[2];
        runs        = std::stoi(argv[3]);
        exp_lower   = std::stoi(argv[4]);
        exp_upper   = std::stoi(argv[5]);
        exp_step    = std::stoi(argv[6]);
        p_lower     = std::stoi(argv[7]);
        p_upper     = std::stoi(argv[8]);
        k_value     = std::stoi(argv[9]);
        
        const std::vector<std::string> valid = { "seq_mergesort", "par_mergesort", "seq_kway", "par_kway", "par_brms"};
        if (std::find(valid.begin(), valid.end(), algorithm) == valid.end()){
            std::cerr << "Algoritmo desconocido " << algorithm << std::endl;
            std::exit(EXIT_FAILURE);
        }
        if (runs < 1 || exp_lower > exp_upper || exp_step < 1 || p_lower > p_upper) {
            std::cerr << "Parametros invalidos. Revisa los rangos.\n";
            std::exit(EXIT_FAILURE);
        }
    } catch (const std::exception& e){
        std::cerr << "Error al parsear los argumentos: " << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

std::vector<int> generateArray(int n, unsigned seed = 42) {
    std::vector<int> arr(n);
    std::mt19937 rng(seed ^ static_cast<unsigned>(n));
    std::uniform_int_distribution<int> dist(0, n * 10);
    for (int& x : arr)
        x = dist(rng);
    return arr;
}

bool isSorted(const std::vector<int>& arr) {
    for (size_t i = 1; i < arr.size(); i++)
        if (arr[i] < arr[i-1]) return false;
    return true;
}

void writeCSV(const std::string& filename, const std::vector<Experiment>& results) {
    std::ofstream f(filename);
    f << "algorithm,n,threads,time_avg,time_min,time_max,speedup,efficiency\n";
    for (const auto& r : results) {
        f << r.algorithm << ","
          << r.n << ","
          << r.threads << ","
          << r.time_avg << ","
          << r.time_min << ","
          << r.time_max << ","
          << r.speedup << ","
          << r.efficiency << "\n";
    }
    f.close();
    std::cout << "\tResultados guardados en: " << filename << std::endl;
}

void printExperiment(const Experiment& e) {
    std::string thread_label;
    if (e.threads == -1) {
        thread_label = "serial";
    } else {
        thread_label = std::to_string(e.threads);
    }
    std::cout << " [" << e.algorithm << "]"
              << " n=" << e.n
              << " p=" << thread_label
              << " t_avg=" << e.time_avg  << "s"
              << " speedup=" << e.speedup
              << " efic=" << e.efficiency
              << std::endl;
}

std::tuple<double,double,double> benchmark(
    const std::vector<int>& original,
    const std::string& algorithm,
    int threads,
    int runs,
    int k_value)
{
    std::vector<double> times;
    int left  = 0;
    int right = static_cast<int>(original.size()) - 1;

    for (int r = 0; r < runs; r++) {
        std::vector<int> arr = original;
        double t0 = omp_get_wtime();

        if (algorithm == "seq_mergesort") {
            sequentialMergeSort(arr, left, right);

        } else if (algorithm == "seq_kway") {
            sequentialKWay(arr, left, right, k_value);

        } else if (algorithm == "par_mergesort") {
            omp_set_num_threads(threads);
            #pragma omp parallel
            #pragma omp single
            parallelMergeSort(arr, left, right);

        } else if (algorithm == "par_kway") {
            omp_set_num_threads(threads);
            #pragma omp parallel
            #pragma omp single
            parallelKWay(arr, left, right, k_value);

        } else if (algorithm == "par_brms") {
            omp_set_num_threads(threads);
            #pragma omp parallel
            #pragma omp single
            parallelBRMS(arr, left, right);
        }

        double t1 = omp_get_wtime();
        times.push_back(t1 - t0);

        if (r == 0 && !isSorted(arr))
            std::cerr << "Arreglo no ordenado" << std::endl;
    }

    double avg = std::accumulate(times.begin(), times.end(), 0.0) / runs;
    double mn = *std::min_element(times.begin(), times.end());
    double mx = *std::max_element(times.begin(), times.end());
    return {avg, mn, mx};
}

int main(int argc, char* argv[]){
    std::string output_file, algorithm;
    int runs, exp_lower, exp_upper, exp_step, p_lower, p_upper, k_value;
    validateInput(argc, argv, output_file, algorithm, runs, exp_lower, exp_upper, exp_step, p_lower, p_upper, k_value);

    // Construir rangos de n y p
    std::vector<int> sizes;
    for (int e = exp_lower; e <= exp_upper; e += exp_step)
        sizes.push_back(1 << e);

    std::vector<int> threadCounts;
    for (int e = p_lower; e <= p_upper; e++)
        threadCounts.push_back(1 << e);

    bool isParallel = (algorithm == "par_mergesort" || algorithm == "par_kway" || algorithm == "par_brms");

    std::string serial_algo;
    if (algorithm == "par_mergesort") serial_algo = "seq_mergesort";
    else if (algorithm == "par_kway") serial_algo = "seq_kway";
    else if (algorithm == "par_brms") serial_algo = "seq_mergesort";
    else serial_algo = algorithm;

    std::cout << "Algoritmo: " << algorithm << std::endl;
    std::cout << "Procesadores: " << omp_get_num_procs() << std::endl;
    std::cout << "Repeticiones: " << runs << std::endl;
    std::cout << "Tamanos n: ";
    for (int s : sizes) std::cout << s << " "; std::cout << std::endl;
    std::cout << "Hebras p: ";
    for (int p : threadCounts) std::cout << p << " "; std::cout << std::endl;
    std::cout << "Valor de k: " << k_value << std::endl;

    std::vector<Experiment> results;

    for (int n : sizes) {
        std::cout << "\n-- n = " << n << " --" << std::endl;
        auto original = generateArray(n);

        auto [avg_s, mn_s, mx_s] = benchmark(original, serial_algo, -1, runs, k_value);
        double t_serial = avg_s;

        Experiment es;
        es.algorithm = serial_algo;
        es.n = n;
        es.threads = -1;
        es.time_avg = avg_s;
        es.time_min = mn_s;
        es.time_max = mx_s;
        es.speedup = 1.0;
        es.efficiency = 1.0;
        results.push_back(es);
        printExperiment(es);

        // Paralelo con cada p
        if (isParallel) {
            for (int p : threadCounts) {
                auto [avg_p, mn_p, mx_p] = benchmark(original, algorithm, p, runs, k_value);

                Experiment ep;
                ep.algorithm = algorithm;
                ep.n = n;
                ep.threads = p;
                ep.time_avg = avg_p;
                ep.time_min = mn_p;
                ep.time_max = mx_p;
                ep.speedup = t_serial / avg_p;
                ep.efficiency = ep.speedup / p;
                results.push_back(ep);
                printExperiment(ep);
            }
        }
    }

    writeCSV(output_file, results);
    return 0;
}
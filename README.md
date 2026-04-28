# T1: Algoritmos de Ordenamiento: Implementaciones Secuenciales y Paralelas

## Integrantes - Grupo 5

| Nombre | Matrícula | GitHub User |
|--------|-----------|-------------|
| Sofía Ignacia López Aguilera | 2022451769 | lulunkaii |
| Antonia Renata Montero López | 2022416394 | amonlop |
| Jhostian Matías San Martín Morales | 2022429453 | Jhoossmm |



## Descripción

Este repositorio contiene todas las implementaciones de la Tarea 1 de la asignatura 501500-1 Sistemas Distribuidos y Paralelismo (S1-2026), es decir:

* Mergesort secuencial y su versión paralela.
* K-way mergesort secuencial y sus versiones paralelas.
* Mergesort utilizando ranks y búsqueda binaria paralelo.

## Compilación y ejecución

### Requisitos
- g++ con soporte OpenMP
- Python 3
  
#### Instalación de Librerías de Python
```bash
pip install -r requirements.txt
```

### Compilar

```bash
make
```

### Ejecutar un algoritmo

```bash
make <target> [RUNS=5] [EXP_LO=20] [EXP_HI=26] [EXP_ST=2] [P_LO=0] [P_HI=3] [K_VALUE=8]
```

| Target | Algoritmo |
|---|---|
| `seq_ms` | Mergesort secuencial |
| `par_ms` | Mergesort paralelo |
| `seq_kw` | K-way secuencial |
| `par_kw` | K-way paralelo |
| `par_full` | Full K-way con merge paralelo |
| `par_br` | Mergesort con binary ranks |
| `all_experiments` | Todos los anteriores |

**Ejemplo:**
```bash
make par_ms RUNS=5 EXP_LO=20 EXP_HI=26 EXP_ST=2 P_LO=0 P_HI=3 K_VALUE = 8
```
Corre mergesort paralelo con `n ∈ {2²⁰, 2²², 2²⁴, 2²⁶}` y `p ∈ {1, 2, 4, 8}`, con 5 repeticiones por experimento. El argumento `K_VALUE` solo se ocupa en los algoritmos de K-way, pero es necesario colocarlo como argumento en todos los algoritmos debido a la estructura del main.cpp.

### Parámetros

| Parámetro | Descripción | Ejemplo |
|---|---|---|
| `RUNS` | Repeticiones por experimento | `5` |
| `EXP_LO` | Exponente mínimo de n | `20` → n = 2²⁰ |
| `EXP_HI` | Exponente máximo de n | `26` → n = 2²⁶ |
| `EXP_ST` | Salto entre exponentes | `2` → ×2² entre cada n |
| `P_LO` | Exponente mínimo de hebras | `0` → p = 1 |
| `P_HI` | Exponente máximo de hebras | `3` → p = 8 |
| `K_VALUE` | Valor de K para K-way | `8` → k = 8 |

### Resultados

Los CSV se guardan en `alg_results/`. Al correr `all_experiments` se generan los CSV de los 6 algoritmos.
Por otro lado, al ejecutar el make, automáticamente se hace profiling (perf stat) del algoritmo en ejecución. Estos resultados se guardan en `perf_outputs/`.
Para los plots, `plots.py` recorre los resultados obtenidos de los CSV de los algoritmos y guarda los plots en `plots_results/`




# T1: Algoritmos de Ordenamiento: Implementaciones Secuenciales y Paralelas

## Integrantes - Grupo 5

| Nombre | Matrícula | GitHub User |
|--------|-----------|-------------|
| Sofía Ignacia López Aguilera | 2022451769 | lulunkaii |
| Antonia Renata Montero López | XXXXXXXXXX | amonlop |
| Jhostian Matías San Martín Morales | XXXXXXXXXX | Jhoossmm |



## Descripción

Este repositorio contiene todas las implementaciones de la Tarea 1 de la asignatura 501500-1 Sistemas Distribuidos y Paralelismo (S1-2026), es decir:

* Mergesort secuencial y su versión paralela.
* K-way mergesort secuencial y su versión paralela.
* Mergesort utilizando ranks y búsqueda binaria paralelo.

## Compilación y ejecución

### Requisitos
- g++ con soporte OpenMP
- Python 3 con `pandas` instalado (solo para consolidar resultados)

### Compilar

```bash
make
```

### Ejecutar un algoritmo

```bash
make <target> [RUNS=5] [EXP_LO=20] [EXP_HI=26] [EXP_ST=2] [P_LO=0] [P_HI=3]
```

| Target | Algoritmo |
|---|---|
| `seq_ms` | Mergesort secuencial |
| `par_ms` | Mergesort paralelo |
| `seq_kw` | K-way secuencial |
| `par_kw` | K-way paralelo |
| `par_br` | BRMS paralelo (baseline: mergesort secuencial) |
| `all_experiments` | Todos los anteriores + CSV consolidado |

**Ejemplo:**
```bash
make par_ms RUNS=5 EXP_LO=20 EXP_HI=26 EXP_ST=2 P_LO=0 P_HI=3
```
Corre mergesort paralelo con `n ∈ {2²⁰, 2²², 2²⁴, 2²⁶}` y `p ∈ {1, 2, 4, 8}`, con 5 repeticiones por experimento.

### Parámetros

| Parámetro | Descripción | Ejemplo |
|---|---|---|
| `RUNS` | Repeticiones por experimento | `5` |
| `EXP_LO` | Exponente mínimo de n | `20` → n = 2²⁰ |
| `EXP_HI` | Exponente máximo de n | `26` → n = 2²⁶ |
| `EXP_ST` | Salto entre exponentes | `2` → ×2² entre cada n |
| `P_LO` | Exponente mínimo de hebras | `0` → p = 1 |
| `P_HI` | Exponente máximo de hebras | `3` → p = 8 |

### Resultados

Los CSV se guardan en `output/`. Al correr `all_experiments` se genera además `output/all_results.csv` con todos los datos consolidados para graficar.




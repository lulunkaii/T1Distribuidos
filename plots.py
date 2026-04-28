import os
import glob
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

def generate_plots():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    # 1. Definir rutas absolutas uniendo la carpeta del script con los directorios
    input_dir = os.path.join(script_dir, "alg_results")
    output_dir = os.path.join(script_dir, "plots_results")
    
    print(f"Buscando CSVs en: {input_dir}")
    
    # Crear la carpeta de salida si no existe
    os.makedirs(output_dir, exist_ok=True)

    # Leer y consolidar todos los CSVs
    all_files = glob.glob(os.path.join(input_dir, "*.csv"))
    if not all_files:
        print(f"Error: No se encontraron archivos .csv en {input_dir}")
        print(f"Archivos presentes en la carpeta del script: {os.listdir(script_dir)}")
        return

    df_list = [pd.read_csv(f) for f in all_files]
    df_full = pd.concat(df_list, ignore_index=True)

    # 2. Limpiar datos: Eliminar filas duplicadas
    # Como seq_mergesort y seq_kway se evalúan múltiples veces en distintos CSVs,
    # nos quedamos con una sola copia por cada combinación de (algoritmo, n, threads).
    df_clean = df_full.drop_duplicates(subset=['algorithm', 'n', 'threads']).copy()

    # 3. Separar algoritmos Secuenciales de Paralelos
    algos_secuenciales = ['seq_mergesort', 'seq_kway']
    algos_paralelos = ['par_mergesort', 'par_brms', 'par_kway', 'par_full']

    df_seq = df_clean[df_clean['algorithm'].isin(algos_secuenciales)].copy()
    df_par = df_clean[df_clean['algorithm'].isin(algos_paralelos)].copy()

    # Configuración visual de los gráficos
    sns.set_theme(style="whitegrid", palette="deep")
    
    # Obtener el tamaño de arreglo más grande para las comparativas de paralelismo
    max_n = df_clean['n'].max()
    df_par_max = df_par[df_par['n'] == max_n].copy()

    print(f"Generando gráficos para tamaño máximo n = {max_n}...")

    # =========================================================
    # GRÁFICO 1: Comparativa Secuencial (Gráfico de Barras)
    # =========================================================
    plt.figure(figsize=(10, 6))
    sns.barplot(data=df_seq, x='n', y='time_avg', hue='algorithm')
    plt.title('Comparación de Algoritmos Secuenciales (Tiempo vs Tamaño n)')
    plt.xlabel('Tamaño del Arreglo (n)')
    plt.ylabel('Tiempo Promedio (s)')
    # Usar escala logarítmica en Y si los tiempos varían drásticamente entre n pequeños y grandes
    plt.yscale('log') 
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, '01_secuenciales_tiempo_barras.png'))
    plt.close()

    # =========================================================
    # GRÁFICO 2: Comparativa Paralelos (Gráfico de Barras)
    # =========================================================
    plt.figure(figsize=(10, 6))
    sns.barplot(data=df_par_max, x='threads', y='time_avg', hue='algorithm')
    plt.title(f'Tiempos de Ejecución Algoritmos Paralelos (n = {max_n})')
    plt.xlabel('Número de Hebras (Threads)')
    plt.ylabel('Tiempo Promedio (s)')
    plt.legend(title='Algoritmo')
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, '02_paralelos_tiempo_barras.png'))
    plt.close()

    # =========================================================
    # GRÁFICO 3: Speedup (Gráfico de Líneas)
    # =========================================================
    plt.figure(figsize=(10, 6))
    sns.lineplot(data=df_par_max, x='threads', y='speedup', hue='algorithm', marker='o', linewidth=2)
    
    # Añadir línea de Speedup Ideal (Lineal)
    threads_unique = sorted(df_par_max['threads'].unique())
    plt.plot(threads_unique, threads_unique, 'k--', label='Speedup Ideal (Lineal)')
    
    plt.title(f'Speedup vs Número de Hebras (n = {max_n})')
    plt.xlabel('Número de Hebras (Threads)')
    plt.ylabel('Speedup (T_sec / T_par)')
    plt.legend(title='Algoritmo')
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, '03_paralelos_speedup.png'))
    plt.close()

    # =========================================================
    # GRÁFICO 4: Eficiencia (Gráfico de Líneas)
    # =========================================================
    plt.figure(figsize=(10, 6))
    sns.lineplot(data=df_par_max, x='threads', y='efficiency', hue='algorithm', marker='s', linewidth=2)
    
    # Añadir línea de Eficiencia Ideal (1.0)
    plt.axhline(y=1.0, color='k', linestyle='--', label='Eficiencia Ideal (1.0)')
    
    plt.title(f'Eficiencia vs Número de Hebras (n = {max_n})')
    plt.xlabel('Número de Hebras (Threads)')
    plt.ylabel('Eficiencia (Speedup / Threads)')
    plt.ylim(0, 1.1)
    plt.legend(title='Algoritmo')
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, '04_paralelos_eficiencia.png'))
    plt.close()

    print(f"¡Listo! 4 gráficos han sido guardados en la carpeta '{output_dir}/'")

if __name__ == "__main__":
    generate_plots()
##agregar esta linea si tira error en la ejecución de perf
##sudo sysctl -w kernel.perf_event_paranoid=-1
##sudo sysctl -w kernel.kptr_restrict=0


##Si no quieres que se ejecute dos veces cada vez que hagas un test en un algoritmo paralelo, 
## puedes comentar la línea de perf c2c en el Makefile y dejar solo el perf stat (o viceversa).
CXX      = g++
CXXFLAGS = -O2 -fopenmp -std=c++17 -I algorithms/

TARGET = mergesort
SRCS   = main.cpp \
    algorithms/seq_mergesort.cpp \
    algorithms/parallel_mergesort.cpp \
    algorithms/seq_kway.cpp \
    algorithms/parallel_kway.cpp \
    algorithms/parallel_brms.cpp

# --- Argumentos por defecto ---
RUNS    ?= 5
EXP_LO  ?= 20
EXP_HI  ?= 26
EXP_ST  ?= 2
P_LO    ?= 0
P_HI    ?= 3
K_VALUE ?= 4

# Directorios
OUT_DIR = output
PERF_DIR = perf_outputs

# Eventos de monitoreo
EVENTS = cycles,instructions,cache-references,cache-misses,stalled-cycles-backend

# Comandos de Perf
P_STAT = perf stat -e $(EVENTS) -o $(PERF_DIR)/stat_$@.txt --append
P_C2C  = perf c2c record -o $(PERF_DIR)/c2c_$@.data
P_C2C_REP = perf c2c report -i $(PERF_DIR)/c2c_$@.data --stdio > $(PERF_DIR)/c2c_$@.txt
# Comando para borrar el binario .data
P_RM_DATA = rm -f $(PERF_DIR)/c2c_$@.data

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

init:
	mkdir -p $(OUT_DIR)
	mkdir -p $(PERF_DIR)

# --- Algoritmos Secuenciales ---
seq_ms: $(TARGET) init
	$(P_STAT) ./$(TARGET) $(OUT_DIR)/seq_mergesort.csv seq_mergesort $(RUNS) $(EXP_LO) $(EXP_HI) $(EXP_ST) 0 0

seq_kw: $(TARGET) init
	$(P_STAT) ./$(TARGET) $(OUT_DIR)/seq_kway.csv seq_kway $(RUNS) $(EXP_LO) $(EXP_HI) $(EXP_ST) 0 0 $(K_VALUE)

# --- Algoritmos Paralelos ---
par_ms: $(TARGET) init
	$(P_STAT) ./$(TARGET) $(OUT_DIR)/par_mergesort.csv par_mergesort $(RUNS) $(EXP_LO) $(EXP_HI) $(EXP_ST) $(P_LO) $(P_HI)
	$(P_C2C) ./$(TARGET) $(OUT_DIR)/par_mergesort.csv par_mergesort $(RUNS) $(EXP_LO) $(EXP_HI) $(EXP_ST) $(P_LO) $(P_HI)
	$(P_C2C_REP)
	$(P_RM_DATA)

par_kw: $(TARGET) init
	$(P_STAT) ./$(TARGET) $(OUT_DIR)/par_kway.csv par_kway $(RUNS) $(EXP_LO) $(EXP_HI) $(EXP_ST) $(P_LO) $(P_HI) $(K_VALUE)
	$(P_C2C) ./$(TARGET) $(OUT_DIR)/par_kway.csv par_kway $(RUNS) $(EXP_LO) $(EXP_HI) $(EXP_ST) $(P_LO) $(P_HI) $(K_VALUE)
	$(P_C2C_REP)
	$(P_RM_DATA)

par_br: $(TARGET) init
	$(P_STAT) ./$(TARGET) $(OUT_DIR)/par_brms.csv par_brms $(RUNS) $(EXP_LO) $(EXP_HI) $(EXP_ST) $(P_LO) $(P_HI)
	$(P_C2C) ./$(TARGET) $(OUT_DIR)/par_brms.csv par_brms $(RUNS) $(EXP_LO) $(EXP_HI) $(EXP_ST) $(P_LO) $(P_HI)
	$(P_C2C_REP)
	$(P_RM_DATA)

all_experiments: $(TARGET) init
	@echo "Ejecutando experimentos..."
	$(MAKE) seq_ms
	$(MAKE) par_ms
	$(MAKE) seq_kw
	$(MAKE) par_kw
	$(MAKE) par_br
	python3 merge_results.py

clean:
	rm -f $(TARGET)
	rm -rf $(OUT_DIR)
	rm -rf $(PERF_DIR)
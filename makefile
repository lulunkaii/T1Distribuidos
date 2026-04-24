CXX      = g++
CXXFLAGS = -O2 -fopenmp -std=c++17 -I algorithms/

TARGET = mergesort
SRCS   = main.cpp \
	algorithms/seq_mergesort.cpp \
	algorithms/parallel_mergesort.cpp \
	algorithms/seq_kway.cpp \
	algorithms/parallel_kway.cpp \
	algorithms/parallel_brms.cpp

RUNS    ?= 5
EXP_LO  ?= 20
EXP_HI  ?= 26
EXP_ST  ?= 2
P_LO    ?= 0
P_HI    ?= 3

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	mkdir -p output
	./$(TARGET) output/results.csv par_mergesort $(RUNS) $(EXP_LO) $(EXP_HI) $(EXP_ST) $(P_LO) $(P_HI)

seq_ms: $(TARGET)
	mkdir -p output
	./$(TARGET) output/seq_mergesort.csv seq_mergesort $(RUNS) $(EXP_LO) $(EXP_HI) $(EXP_ST) 0 0

par_ms: $(TARGET)
	mkdir -p output
	./$(TARGET) output/par_mergesort.csv par_mergesort $(RUNS) $(EXP_LO) $(EXP_HI) $(EXP_ST) $(P_LO) $(P_HI)

seq_kw: $(TARGET)
	mkdir -p output
	./$(TARGET) output/seq_kway.csv seq_kway $(RUNS) $(EXP_LO) $(EXP_HI) $(EXP_ST) 0 0

par_kw: $(TARGET)
	mkdir -p output
	./$(TARGET) output/par_kway.csv par_kway $(RUNS) $(EXP_LO) $(EXP_HI) $(EXP_ST) $(P_LO) $(P_HI)

par_br: $(TARGET)
	mkdir -p output
	./$(TARGET) output/par_brms.csv par_brms $(RUNS) $(EXP_LO) $(EXP_HI) $(EXP_ST) $(P_LO) $(P_HI)

all_experiments: $(TARGET)
	mkdir -p output
	./$(TARGET) output/seq_mergesort.csv seq_mergesort $(RUNS) $(EXP_LO) $(EXP_HI) $(EXP_ST) 0 0
	./$(TARGET) output/par_mergesort.csv par_mergesort $(RUNS) $(EXP_LO) $(EXP_HI) $(EXP_ST) $(P_LO) $(P_HI)
	./$(TARGET) output/seq_kway.csv      seq_kway      $(RUNS) $(EXP_LO) $(EXP_HI) $(EXP_ST) 0 0
	./$(TARGET) output/par_kway.csv      par_kway      $(RUNS) $(EXP_LO) $(EXP_HI) $(EXP_ST) $(P_LO) $(P_HI)
	./$(TARGET) output/par_brms.csv      par_brms      $(RUNS) $(EXP_LO) $(EXP_HI) $(EXP_ST) $(P_LO) $(P_HI)
	python3 merge_results.py

clean:
	rm -f $(TARGET) output/*.csv
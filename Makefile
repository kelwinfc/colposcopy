GCC=g++
LBOOST_FLAGS=-lboost_system -lboost_filesystem
GCC_FLAGS= `pkg-config opencv --cflags` -Ilib -Icontrib $(LBOOST_FLAGS) -Wall -O3
MAIN_FLAGS= `pkg-config opencv --cflags --libs` $(LBOOST_FLAGS) -Ilib -Icontrib -Wall -O3

FILES=utils specular_reflection diagnosis_phase
EXECUTABLES=test_specular_reflection test_diagnosis_phase

DEP_utils=
DEP_specular_reflection=utils
DEP_diagnosis_phase=utils specular_reflection
DEP_test_specular_reflection=$(DEP_specular_reflection) specular_reflection
DEP_test_diagnosis_phase=$(DEP_diagnosis_phase) diagnosis_phase

all: $(EXECUTABLES)

test_specular_reflection: $(FILES:%=bin/%.o) bin/test_specular_reflection.o
	$(GCC) $^ -o $@ $(MAIN_FLAGS)

test_diagnosis_phase: $(FILES:%=bin/%.o) bin/test_diagnosis_phase.o
	$(GCC) $^ -o $@ $(MAIN_FLAGS)

#General rule for compiling
bin/%.o: src/%.cpp lib/%.hpp
	$(GCC) -c $< -o $@ $(GCC_FLAGS)

bin/utils.o: $(DEP_utils:%=src/%.cpp) $(DEP_utils:%=lib/%.hpp)
bin/specular_reflection.o: $(DEP_specular_reflection:%=src/%.cpp) $(DEP_specular_reflection:%=lib/%.hpp)
bin/diagnosis_phase.o: $(DEP_diagnosis_phase:%=src/%.cpp) $(DEP_diagnosis_phase:%=lib/%.hpp)

bin/test_specular_reflection.o: $(DEP_test_specular_reflection:%=src/%.cpp) $(DEP_test_specular_reflection:%=lib/%.hpp)
bin/test_diagnosis_phase.o: $(DEP_test_diagnosis_phase:%=src/%.cpp) $(DEP_test_diagnosis_phase:%=lib/%.hpp)

clean:
	rm -rf *~ */*~ */*/*~ *.pyc */*.pyc $(EXECUTABLES) bin/*.o

count_lines:
	wc -l src/*.cpp src/*.py lib/*.hpp other/*.py other/*.cpp | sort -gk 1

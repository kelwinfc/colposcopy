GCC=g++
LBOOST_FLAGS=-lboost_system -lboost_filesystem
GCC_FLAGS= `pkg-config opencv --cflags` -Ilib -Icontrib $(LBOOST_FLAGS) -Wall -O3
MAIN_FLAGS= `pkg-config opencv --cflags --libs` $(LBOOST_FLAGS) -Ilib -Icontrib -Wall -O3

FILES=utils\
	specular_reflection\
	diagnosis_phase diagnosis_phase_feature_extractor diagnosis_phase_distance
EXECUTABLES=test_specular_reflection test_diagnosis_phase

DEP_utils=
DEP_specular_reflection=utils
DEP_diagnosis_phase_feature_extractor=utils
DEP_diagnosis_phase_distance=utils
DEP_diagnosis_phase=utils diagnosis_phase_feature_extractor specular_reflection
DEP_test_specular_reflection=$(DEP_specular_reflection) specular_reflection
DEP_test_diagnosis_phase=$(DEP_diagnosis_phase) diagnosis_phase

all: $(EXECUTABLES)

test_specular_reflection: $(FILES:%=bin/%.o)
	$(GCC) $^ -o $@ $(MAIN_FLAGS) -I"lib/tests" src/tests/$@.cpp

test_diagnosis_phase: $(FILES:%=bin/%.o)
	$(GCC) $^ -o $@ $(MAIN_FLAGS) -I"lib/tests" src/tests/$@.cpp

#General rule for compiling
bin/%.o: src/%.cpp lib/%.hpp
	$(GCC) -c $< -o $@ $(GCC_FLAGS)

bin/utils.o: $(DEP_utils:%=src/%.cpp) $(DEP_utils:%=lib/%.hpp)
bin/specular_reflection.o: $(DEP_specular_reflection:%=src/%.cpp) $(DEP_specular_reflection:%=lib/%.hpp)
bin/diagnosis_phase_feature_extractor.o: $(DEP_diagnosis_phase_feature_extractor:%=src/%.cpp) $(DEP_diagnosis_phase_feature_extractor:%=lib/%.hpp)
bin/diagnosis_phase_distance.o: $(DEP_diagnosis_phase_distance:%=src/%.cpp) $(DEP_diagnosis_phase_distance:%=lib/%.hpp)
bin/diagnosis_phase.o: $(DEP_diagnosis_phase:%=src/%.cpp) $(DEP_diagnosis_phase:%=lib/%.hpp)

clean:
	rm -rf *~ */*~ */*/*~ *.pyc */*.pyc $(EXECUTABLES) bin/*.o

count_lines:
	wc -l src/*.cpp src/*.py lib/*.hpp other/*.py other/*.cpp | sort -gk 1

update_ignore:
	(cat -v .gitignore.base; echo $(EXECUTABLES)) > .gitignore;

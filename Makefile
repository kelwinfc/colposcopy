GCC=g++
LBOOST_FLAGS=-lboost_system -lboost_filesystem
GCC_FLAGS= `pkg-config opencv --cflags` -Ilib -Icontrib $(LBOOST_FLAGS) -Wall -O3
MAIN_FLAGS= `pkg-config opencv --cflags --libs` $(LBOOST_FLAGS) -Ilib -Icontrib -Wall -O3

FILES=utils\
	specular_reflection\
	feature_extractor distance classifier\
	diagnosis_phase\
	ranking

EXECUTABLES=test_specular_reflection test_diagnosis_phase test_classifiers\
	test_motion generate_pairs_of_images_to_annotate test_ranking_abc

DEP_utils=
DEP_specular_reflection=utils
DEP_feature_extractor=utils
DEP_distance=utils
DEP_classifier=utils feature_extractor
DEP_diagnosis_phase=utils feature_extractor distance classifier specular_reflection
DEP_ranking=utils

DEP_test_specular_reflection=$(DEP_specular_reflection) specular_reflection
DEP_test_diagnosis_phase=$(DEP_diagnosis_phase) diagnosis_phase
DEP_test_classifiers=$(DEP_neighbors)
DEP_test_motion=$(DEP_diagnosis_phase)
DEP_generate_pairs_of_images_to_annotate=$(DEP_diagnosis_phase)
DEP_test_ranking_abc=$(DEP_ranking)

all: $(EXECUTABLES)

test_specular_reflection: $(FILES:%=bin/%.o)
	$(GCC) $^ -o $@ $(MAIN_FLAGS) -I"lib/tests" src/tests/$@.cpp

test_diagnosis_phase: $(FILES:%=bin/%.o) src/tests/test_diagnosis_phase.cpp lib/tests/test_diagnosis_phase.hpp
	$(GCC) $^ -o $@ $(MAIN_FLAGS) -I"lib/tests"

test_classifiers: $(FILES:%=bin/%.o) src/tests/test_classifiers.cpp lib/tests/test_classifiers.hpp
	$(GCC) $^ -o $@ $(MAIN_FLAGS) -I"lib/tests"

test_motion: $(FILES:%=bin/%.o) src/tests/test_motion.cpp lib/tests/test_motion.hpp
	$(GCC) $^ -o $@ $(MAIN_FLAGS) -I"lib/tests"

test_ranking_abc: $(FILES:%=bin/%.o) src/tests/test_ranking_abc.cpp lib/tests/test_ranking_abc.hpp
	$(GCC) $^ -o $@ $(MAIN_FLAGS) -I"lib/tests"

generate_pairs_of_images_to_annotate: $(FILES:%=bin/%.o) src/generate_pairs_of_images_to_annotate.cpp lib/generate_pairs_of_images_to_annotate.hpp
	$(GCC) $^ -o $@ $(MAIN_FLAGS) -I"lib/tests"

#General rule for compiling
bin/%.o: src/%.cpp lib/%.hpp
	$(GCC) -c $< -o $@ $(GCC_FLAGS)

bin/utils.o: $(DEP_utils:%=src/%.cpp) $(DEP_utils:%=lib/%.hpp)
bin/specular_reflection.o: $(DEP_specular_reflection:%=src/%.cpp) $(DEP_specular_reflection:%=lib/%.hpp)
bin/feature_extractor.o: $(DEP_feature_extractor:%=src/%.cpp) $(DEP_feature_extractor:%=lib/%.hpp)
bin/distance.o: $(DEP_distance:%=src/%.cpp) $(DEP_distance:%=lib/%.hpp)
bin/diagnosis_phase.o: $(DEP_diagnosis_phase:%=src/%.cpp) $(DEP_diagnosis_phase:%=lib/%.hpp)
bin/classifier.o: $(DEP_classifier:%=src/%.cpp) $(DEP_classifier:%=lib/%.hpp)
bin/ranking.o: $(DEP_ranking:%=src/%.cpp) $(DEP_ranking:%=lib/%.hpp)

clean:
	rm -rf *~ */*~ */*/*~ *.pyc */*.pyc $(EXECUTABLES) bin/*.o

count_lines:
	wc -l src/*.cpp src/*.py lib/*.hpp other/*.py other/*.cpp | sort -gk 1

update_ignore:
	(cat -v .gitignore.base; echo $(EXECUTABLES)) | tr " " "\n" > .gitignore;

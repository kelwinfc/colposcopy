GCC=g++
LBOOST_FLAGS=-lboost_system -lboost_filesystem
GCC_FLAGS= `pkg-config opencv --cflags` -Ilib -Icontrib $(LBOOST_FLAGS) -Wall -O3
MAIN_FLAGS= `pkg-config opencv --cflags --libs` $(LBOOST_FLAGS) -Ilib -Icontrib -Wall -O3

FILES=specular_reflection utils
EXECUTABLES=test_specular_reflection

DEP_utils=
DEP_specular_reflection=utils
DEP_test_specular_reflection=utils specular_reflection

all: $(EXECUTABLES)

test_specular_reflection: $(FILES:%=bin/%.o) bin/test_specular_reflection.o
	$(GCC) $^ -o $@ $(MAIN_FLAGS)

#General rule for compiling
bin/%.o: src/%.cpp lib/%.hpp
	$(GCC) -c $< -o $@ $(GCC_FLAGS)

bin/utils.o: $(DEP_utils:%=src/%.cpp) $(DEP_utils:%=lib/%.hpp)
bin/specular_reflection.o: $(DEP_specular_reflection:%=src/%.cpp) $(DEP_specular_reflection:%=lib/%.hpp)

bin/test_specular_reflection.o: $(DEP_test_specular_reflection:%=src/%.cpp) $(DEP_test_specular_reflection:%=lib/%.hpp)

clean:
	rm -rf *~ */*~ */*/*~ *.pyc */*.pyc $(EXECUTABLES) bin/*.o

count_lines:
	wc -l src/*.cpp src/*.py lib/*.hpp other/*.py other/*.cpp | sort -gk 1

GCC=g++

LBOOST_FLAGS=-lboost_system -lboost_filesystem
GCC_FLAGS= `pkg-config opencv --cflags` -Ilib -Icontrib $(LBOOST_FLAGS) -Wall -O3
MAIN_FLAGS= `pkg-config opencv --cflags --libs` $(LBOOST_FLAGS) -Ilib -Icontrib -Wall -O3

FILES=anonadado utils
EXECUTABLES=test_anonadado

DEP_utils=
DEP_anonadado=utils
DEP_test_anonadado=

all: $(EXECUTABLES)

test_anonadado: $(FILES:%=%.o) test_anonadado.o
	$(GCC) $^ -o $@ $(MAIN_FLAGS)

#General rule for compiling
%.o: %.cpp %.hpp
	$(GCC) -c $< -o $@ $(GCC_FLAGS)

bin/utils.o: $(DEP_utils:%=%.cpp) $(DEP_utils:%=%.hpp)
bin/anonadado.o: $(DEP_anonadado:%=%.cpp) $(DEP_anonadado:%=%.hpp)
bin/test_anonadado.o: $(DEP_test_anonadado:%=%.cpp) $(DEP_test_anonadado:%=%.hpp)

clean:
	rm -rf *~ $(EXECUTABLES)

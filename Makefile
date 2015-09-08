CXX=clang++
CXXFLAGS=-std=c++11 -DBOOST_MPL_LIMIT_LIST_SIZE=30 -DBOOST_MPL_CFG_NO_PREPROCESSED_HEADERS $(shell mpicxx --showme:compile) -g

# Profiling:
#CXXFLAGS=-pg $(CXXFLAGS)

OBJS=src/term.o src/unification.o src/printer.o src/runtime/environment.o src/runtime/interpreter.o

all: .depend agentspirit agentspirit-mpi test

agentspirit: $(OBJS) src/main.o
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) src/main.o $(LDFLAGS)

agentspirit-mpi: $(OBJS) src/main-mpi.o
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) src/main-mpi.o -lboost_serialization -lboost_mpi $(shell mpicxx --showme:link) $(LDFLAGS)

test: $(OBJS) src/test.o
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) src/test.o -lboost_serialization $(LDFLAGS)
	./test


.PHONY: clean
clean:
	-rm src/main.o
	-rm src/test.o
	-rm $(OBJS)
	-rm test
	-rm agentspirit

.PHONY: .depend
.depend:
	$(CXX) $(CXXFLAGS) $(DEPENDFLAGS) -MM $(OBJS:.o=.cc) src/main.cc src/test.cc src/main-mpi.cc > $@

-include .depend

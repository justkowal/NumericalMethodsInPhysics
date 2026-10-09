CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
BLAS_LIBS ?= -lblas -lm
.PHONY: help basic activity1 activity2 activity3 activity4 activity4-blas check-env clean
help:
	@echo 'Build: make basic. Start by implementing include/student_tasks.hpp.'
	@echo 'Run ./bin/activity1 then activity2; unfinished work exits 2 (NOT IMPLEMENTED).'
	@echo 'CBLAS branch: make check-env; make activity4-blas. No system installation is performed.'
bin:
	mkdir -p bin
bin/activity%: src/activity%.cpp include/student_tasks.hpp include/student_support.hpp | bin
	$(CXX) $(CXXFLAGS) -Iinclude $< -o $@
basic: activity1 activity2 activity3 activity4
activity1: bin/activity1
activity2: bin/activity2
activity3: bin/activity3
activity4: bin/activity4
check-env:
	@CXX="$(CXX)" BLAS_LIBS="$(BLAS_LIBS)" bash scripts/check_env.sh
activity4-blas: check-env bin/activity4_blas
bin/activity4_blas: src/activity4.cpp include/student_tasks.hpp include/student_support.hpp | bin
	$(CXX) $(CXXFLAGS) -DSTUDENT_WITH_BLAS -Iinclude $< -o $@ $(BLAS_LIBS)
clean:
	rm -rf bin
	@echo 'Data, plots, reports and your source changes were preserved.'

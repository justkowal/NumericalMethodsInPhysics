# NMP Project 01 - INSTRUCTOR REFERENCE. No installation or network actions.
CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
CPPFLAGS ?= -Iinclude
BLAS_LIBS ?= -lblas -lm
BASIC = 01_vector_basics 02_matrix_storage 03_matrix_vector 04_gamma_iterations 05_matrix_matrix_manual 07_validate_compare 08_benchmark
EXTRAS = 09_float_vs_double 10_summation_order
HEADERS = $(wildcard include/nmp/*.hpp)
.PHONY: help check-env basic test results blas test-blas benchmark benchmark-full extras plots clean
help:
	@echo 'Instructor/reference package. Start: make basic; make test; make results; make plots.'
	@echo 'CBLAS branch: make check-env; make blas; make test-blas; make benchmark.'
	@echo 'Optional: make extras. Existing result directories are never silently overwritten.'
	@echo 'Full n=2000 cubic run: make benchmark-full ALLOW_LARGE=1. Do not run unapproved jobs on shared servers.'
check-env:
	@CXX="$(CXX)" BLAS_LIBS="$(BLAS_LIBS)" bash scripts/check_env.sh
bin:
	mkdir -p bin
bin/%: src/%.cpp $(HEADERS) | bin
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@
basic: $(addprefix bin/,$(BASIC))
extras: $(addprefix bin/,$(EXTRAS))
bin/test_core: tests/test_core.cpp $(HEADERS) | bin
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@
test: bin/test_core
	./bin/test_core
results: basic test
	./bin/04_gamma_iterations --n 2000 --steps 30 --gamma both --output results/official
blas: check-env bin/06_matrix_matrix_blas bin/08_benchmark_blas bin/test_blas
bin/06_matrix_matrix_blas: src/06_matrix_matrix_blas.cpp $(HEADERS) | bin
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@ $(BLAS_LIBS)
bin/08_benchmark_blas: src/08_benchmark.cpp $(HEADERS) | bin
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DNMP_WITH_BLAS $< -o $@ $(BLAS_LIBS)
bin/test_blas: tests/test_blas.cpp $(HEADERS) | bin
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@ $(BLAS_LIBS)
test-blas: blas
	./bin/test_blas
benchmark: test-blas
	./bin/08_benchmark_blas --n 128 --repeats 5 --output results/benchmarks/n128
	./bin/08_benchmark_blas --n 256 --repeats 5 --output results/benchmarks/n256
	./bin/08_benchmark_blas --n 512 --repeats 5 --output results/benchmarks/n512
benchmark-full: test-blas
	@test "$(ALLOW_LARGE)" = "1" || (echo 'Set ALLOW_LARGE=1 explicitly, after estimating cost and checking resource policy.'; exit 2)
	./bin/08_benchmark_blas --n 2000 --gamma both --repeats 2 --allow-large --output results/benchmarks/n2000
plots:
	python3 plotting/plot_norms.py results/official --output figures
clean:
	rm -rf bin
	@echo 'Removed binaries only. Numerical results and documents were preserved.'

# GNU/Linux demonstration. Override flags for a confirmed alternative installation.
CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2
CPPFLAGS ?=
LDFLAGS ?=
BLAS_LIBS ?= -lblas -lm
LAPACKE_LIBS ?= -llapacke -llapack -lblas -lm
FFTW_LIBS ?= -lfftw3 -lm

.PHONY: all check clean
all: hello test_blas test_lapacke test_fftw
hello: hello.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@ $(LDFLAGS)
test_blas: test_blas.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@ $(LDFLAGS) $(BLAS_LIBS)
test_lapacke: test_lapacke.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@ $(LDFLAGS) $(LAPACKE_LIBS)
test_fftw: test_fftw.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@ $(LDFLAGS) $(FFTW_LIBS)
check: all
	./hello
	./test_blas
	./test_lapacke
	./test_fftw
clean:
	rm -f hello test_blas test_lapacke test_fftw

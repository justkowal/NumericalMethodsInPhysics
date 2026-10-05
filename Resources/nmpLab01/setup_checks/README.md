# Ungraded environment checks

These four small original C++ programs test compilation and selected library calls. They do not solve any assessed course project.

Use `make hello`, `make test_blas`, `make test_lapacke`, `make test_fftw`, or `make check` in this directory. GNU/Linux default link flags are in the Makefile and the Student Setup Guide. Commands may need adjustment for a separately verified provider/platform.

Expected results:
- hello: `C++ setup OK`, status 0.
- BLAS: dot product 32, then PASS, status 0.
- LAPACKE: solution (2,3), small residual, PASS, status 0.
- FFTW: a four-point impulse transforms to all ones, and the normalized inverse recovers it; errors below 1e-12, PASS, status 0.

**Actual production tests:** hello and BLAS compiled and ran on the authoring Debian 13 container with GNU g++ 14.2.0. LAPACKE and FFTW compilation was attempted but failed because their development headers were absent; neither binary was run. See the verification report for evidence. This is not testing on Taurus or the classroom PCs.

The expected values were checked independently by arithmetic and the finite DFT definition. They are not fabricated runtime output for unavailable libraries.

Never install packages on managed university machines without authorization. The optional `clean` target removes only generated executables. The distribution contains source, not compiled binaries. No external font, textbook or numerical-library binary is included.

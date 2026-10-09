# Project 01 — student starter

This is intentionally NOT a completed project. Work individually under your instructor's assistance policy. The assessed kernels and iteration logic are in `include/student_tasks.hpp`; incomplete functions throw `NOT IMPLEMENTED` and exit with status 2. Compilation alone is not completion. Activity 4 includes an additional labelled driver/timing task in its source.

Read `Simple_Numerical_Operations_with_Vectors_and_Matrices.pdf` first. 
Your setup environment and the numerical project are different tasks. 
No installation is performed by the build system.

## Start from this directory

```bash
make basic
./bin/activity1
./bin/activity2
./bin/activity3 --help
```

The first two commands that run unfinished activities are expected to fail until you implement the corresponding tasks. Rebuild after every source edit. Implement A1 before A2 and A3. Initial small-case answers are supplied as checks, not as a solution to the full project.

## After completing the matrix and iteration functions

```bash
./bin/activity3 --n 2 --steps 30 --gamma both --output data/check_n2
./bin/activity3 --n 2000 --steps 30 --gamma both --output data/official
python3 plotting/plot_norms.py data/official --output plots
```

In the 2-by-2 test both gamma cases decay: compare with the analytic expression in the worksheet. The official size is 2000, not 2. Preserve exactly 30 post-multiplication rows per case, without normalizing iterates. Use a new output directory for a new run or explicitly supply `--overwrite`; your previous data is never silently deleted. Record compiler, system, parameters and implementation choices separately in your own README.

## CBLAS branch

```bash
make check-env
make activity4-blas
./bin/activity4_blas
```

The probe checks only the environment. Implement both mathematical functions first; the tiny A-squared check is followed by your own full-size and timing extension. `BLAS_LIBS` can be overridden to match a verified installation. Do not install packages on Taurus or classroom PCs. A missing library is a blocker to report, not a grading exemption. Complete available manual work while the instructor resolves access.

For large multiplication start with small and moderate sizes. Request instructor guidance before running full cubic benchmarks on a shared machine; do not launch uncontrolled multithreaded jobs. Repeating vector multiplication is much cheaper than repeatedly squaring a dense matrix.

## What to submit

Use the course's correct UPeL group: source files and requested numerical data as a genuine ZIP activity submission; the scientific report as a separate PDF. The public course rule is seven days after the relevant laboratory, with a 20-point deduction per subsequent week of delay. Confirm assignment-specific dates in UPeL with the instructor. The report guide and empty report template are provided; no new official weights or permissions are introduced.

The slide PDF is a teaching reference. Its appendix is optional study, not additional compulsory work. No instructor solutions, master slide TEX, hidden notes, full reference programs or reference result directories are in this starter package.

## Troubleshooting

A compile error about `cblas.h` means the include interface is not visible; a linker error means linkage or libraries need checking. A wrong tiny result usually warrants checking dimensions, stride, accumulation and input/output aliasing before scaling. A non-finite value is a failed computation, not something to hide in a plot. Record the machine, current directory, exact command and first error message. `make clean` removes binaries only.

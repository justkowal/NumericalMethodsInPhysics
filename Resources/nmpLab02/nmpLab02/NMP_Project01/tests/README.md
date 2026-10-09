# Test route and declared scope

Run from the instructor/reference root after checking the active machine. Standard compiler flags are in Makefile. All commands below were exercised either on the production tree or on a fresh source copy; archive-specific reruns are recorded in the verification directory.

```bash
make basic extras
make test
make check-env
make blas
make test-blas
```
The first numerical suite covers V01–V08 and V11–V13. The second covers V09/V10. A library-free `make basic` and `make test` do not need numerical libraries. The actual probe must pass before treating BLAS as present.

The official stored snapshots support all-step full-vector verification:
```bash
python3 tests/verify_numerics.py
python3 tests/test_failures.py
```
The verifier independently traverses Toeplitz diagonals (V14) and constructs converged Decimal oracles at n8/n16 (V15). It writes designated validation outputs. It requires `bin/04_gamma_iterations`, NumPy and the full official snapshots, not merely norm files. In a fresh source-only folder, generate these with `./bin/04_gamma_iterations --n 2000 --steps 30 --gamma both --snapshots --output results/official` first. The failure tests intentionally require existing official output to test collision rejection. They also exercise NaN rejection and a deliberately nonexistent BLAS library. Expected failure is reported as PASS only when the expected exit code occurs.

Optional study checks use programs 09/10 and `scripts/compiler_study.py`; the latter writes and validates every n128 matrix entry in a separate strict process (V16/V17). ASan/UBSan were run on the small core tests:
```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -O1 -g \
  -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude \
  tests/test_core.cpp -o bin/test_core_sanitized
./bin/test_core_sanitized
```
The instructor-only `slide_examples.cpp` compiles the actual showcased mathematical/code anchors with CBLAS. V18 is the clean-source and clean-archive reproducibility audit, not an additional numerical method. See `Test_Coverage.csv`, `Clean_Code_and_Handout_Checks.json` and `Archive_Extraction_Checks.json` for evidence.

Default double product comparison uses `atol=1e-13`, `rtol=1e-12` with the combined criterion `absolute <= atol+rtol*reference_norm`. Full-size independent matvec comparisons use `atol=1e-12`, `rtol=1e-11` because they deliberately change summation order across 2000 terms and 30 steps. Reference convergence was checked at 80 versus 120 decimal digits, not assumed from the name of a type. Zero reference norms make a relative metric undefined; absolute criteria still apply. Passing tests is finite evidence, not a proof for every machine/input.

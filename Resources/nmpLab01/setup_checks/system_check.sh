#!/usr/bin/env bash

set -u

echo "========================================"
echo " Numerical Methods in Physics"
echo " Environment Check"
echo "========================================"

echo
echo "[OS]"
if [ -r /etc/os-release ]; then
    cat /etc/os-release
else
    echo "/etc/os-release not available"
fi

echo
echo "[ARCHITECTURE]"
uname -m

echo
echo "[KERNEL]"
uname -a

echo
echo "[COMPILERS]"
gcc --version 2>/dev/null | head -n 1 || echo "gcc: not found"
g++ --version 2>/dev/null | head -n 1 || echo "g++: not found"

echo
echo "[HEADERS]"
for h in cblas.h lapacke.h fftw3.h; do
    result=$(find /usr/include /usr/local/include \
        -name "$h" 2>/dev/null | head -n 1)

    if [ -n "$result" ]; then
        echo "$h: $result"
    else
        echo "$h: NOT FOUND"
    fi
done

echo
echo "[LIBRARIES]"
ldconfig -p 2>/dev/null \
    | grep -Ei 'blas|openblas|lapack|lapacke|fftw' \
    || echo "No matching ldconfig entries found"

echo
echo "[PYTHON]"
python3 --version 2>/dev/null || echo "python3: not found"

python3 - <<'PY'
modules = ["numpy", "scipy", "matplotlib"]

for module in modules:
    try:
        m = __import__(module)
        print(f"{module}: {getattr(m, '__version__', 'available')}")
    except Exception as exc:
        print(f"{module}: NOT AVAILABLE ({exc})")
PY

echo
echo "Environment check complete."

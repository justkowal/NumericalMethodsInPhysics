#!/usr/bin/env bash
# Read-only discovery, followed by a disposable user-space compile/link/run probe.
set -eu
printf '%s\n' 'Machine category: record locally / Taurus / classroom; this script cannot infer it.'
uname -srm
command -v "${CXX:-g++}"
"${CXX:-g++}" --version | head -n 1
if command -v ldconfig >/dev/null 2>&1; then LDCONFIG=$(command -v ldconfig)
elif [ -x /sbin/ldconfig ]; then LDCONFIG=/sbin/ldconfig
else LDCONFIG=''; fi
if [ -n "$LDCONFIG" ]; then
  "$LDCONFIG" -p | awk '/libblas|openblas/ {print}'
else printf '%s\n' 'ldconfig unavailable: not evidence that BLAS is absent.'; fi
PROBE=$(mktemp -d)
trap 'rm -rf -- "$PROBE"' EXIT
cat > "$PROBE/probe.cpp" <<'CPP'
#include <cblas.h>
#include <iostream>
int main(){double a[4]={1,2,3,4},b[4]={5,6,7,8},c[4]={0,0,0,0};
cblas_dgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,2,2,2,1,a,2,b,2,0,c,2);
if(c[0]!=19||c[1]!=22||c[2]!=43||c[3]!=50)return 1;
std::cout<<"CBLAS compile/link/run numerical probe PASS: 19 22; 43 50\n";}
CPP
# BLAS_LIBS is intentionally split into compiler arguments, not evaluated as shell code.
if "${CXX:-g++}" -std=c++17 "$PROBE/probe.cpp" -o "$PROBE/probe" ${BLAS_LIBS:--lblas -lm}; then
    "$PROBE/probe"
else printf '%s\n' 'SKIPPED-DEPENDENCY: CBLAS failed. Continue manual work; report managed-machine failures.' >&2;exit 3;fi

# Project 01

## environment

### option 1: nix (recommended)

install nix (with flakes enabled):
```bash
sh <(curl -L https://nixos.org/nix/install) --daemon
```

then enter the development shell:
```bash
direnv allow
# or
nix --extra-experimental-features "nix-command flakes" develop
```

### option 2: manual system dependencies

ensure the following packages are installed on your system:
- c++17 compiler (gcc / clang)
- cmake (>= 3.20)
- blas / cblas library (e.g. `libopenblas-dev` or `blas-devel`)
- python 3 with `numpy` and `matplotlib`

## build and run

```bash
cmake -B build -S .
cmake --build build
make -C build run_all
```
#!/usr/bin/env python3
"""Plot existing validated post-step norm CSVs. Never executes a solver."""
from __future__ import annotations
import argparse, csv, math, pathlib
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

def read_norms(path: pathlib.Path):
    with path.open() as f:
        r = csv.DictReader(f)
        if r.fieldnames != ['iteration', 'norm2']:
            raise ValueError(f'{path}: expected iteration,norm2')
        data = list(r)
    k = [int(r['iteration']) for r in data]
    y = [float(r['norm2']) for r in data]
    if k != list(range(1, len(k) + 1)) or not k:
        raise ValueError('Iterations must be contiguous positive integers')
    if any(not math.isfinite(v) or v <= 0 for v in y):
        raise ValueError('Expected finite positive norms')
    return k, y

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('input', type=pathlib.Path, nargs='?', default=pathlib.Path('data'),
                   help='Directory containing norms_gamma2.csv and norms_gamma3.csv')
    p.add_argument('--output', type=pathlib.Path, default=pathlib.Path('plots'),
                   help='Directory where generated plots are saved (defaults to plots)')
    a = p.parse_args()
    a.output.mkdir(parents=True, exist_ok=True)

    for g in (2, 3):
        x, y = read_norms(a.input / f'norms_gamma{g}.csv')
        fig, ax = plt.subplots(figsize=(8, 4.8))
        ax.plot(x, y, 'o-', markersize=3.5, linewidth=1.5)
        ax.set(
            xlabel='Multiplication index k',
            ylabel=r'$\|x^{(k)}\|_2$ (dimensionless)',
            title=f'Gamma = {g}: unnormalized matrix-vector iteration',
            xlim=(1, max(x))
        )
        if g == 2:
            ax.set_yscale('log')
            ax.set_ylabel(r'$\|x^{(k)}\|_2$ (dimensionless, logarithmic axis)')
        else:
            ax.set_ylim(0, max(y) * 1.08)
        ax.grid(True, alpha=0.25)
        fig.tight_layout()
        for ext in ('pdf', 'svg', 'png'):
            fig.savefig(a.output / f'norms_gamma{g}.{ext}', dpi=180)
        plt.close(fig)

    # combined comparison plot
    k2, y2 = read_norms(a.input / 'norms_gamma2.csv')
    k3, y3 = read_norms(a.input / 'norms_gamma3.csv')

    fig, ax = plt.subplots(figsize=(8, 4.8))
    ax.plot(k2, y2, 'o-', color='#1f77b4', markersize=4, linewidth=1.8, label=r'$\gamma = 2$')
    ax.plot(k3, y3, 's-', color='#d62728', markersize=4, linewidth=1.8, label=r'$\gamma = 3$')
    ax.set_yscale('log')
    ax.set(
        xlabel='Multiplication index k',
        ylabel=r'$\|x^{(k)}\|_2$ (dimensionless, logarithmic axis)',
        title=r'Norm Evolution Comparison: $\gamma = 2$ vs $\gamma = 3$',
        xlim=(1, max(k2))
    )
    ax.grid(True, which='both', alpha=0.25)
    ax.legend(frameon=True, loc='center right')
    fig.tight_layout()
    for ext in ('pdf', 'svg', 'png'):
        fig.savefig(a.output / f'norms_gamma2_vs_gamma3.{ext}', dpi=180)
        fig.savefig(a.output / f'norms_comparison.{ext}', dpi=180)
    plt.close(fig)

    print(f'Saved separate and comparison gamma plots from CSV data to {a.output}')
    return 0

if __name__ == '__main__':
    raise SystemExit(main())

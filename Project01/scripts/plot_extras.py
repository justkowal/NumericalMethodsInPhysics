#!/usr/bin/env python3
"""Create timing and optional precision plots from recorded, validated results."""
from __future__ import annotations
import argparse, csv, math, pathlib
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

def rows(path: pathlib.Path):
    with path.open() as f:
        return list(csv.DictReader(f))

def save(fig, path: pathlib.Path):
    fig.tight_layout()
    for ext in ('pdf', 'svg', 'png'):
        fig.savefig(path.with_suffix('.' + ext), dpi=180)
    plt.close(fig)

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root', type=pathlib.Path, default=pathlib.Path('.'),
                   help='Root directory to search for results/ (defaults to .)')
    p.add_argument('--output', type=pathlib.Path, default=pathlib.Path('plots'),
                   help='Output directory for plots (defaults to plots)')
    a = p.parse_args()
    a.output.mkdir(parents=True, exist_ok=True)

    # benchmark timings plot across dimensions
    measurements = []
    for n in (128, 256, 512, 2000):
        candidates = [
            a.root / f'results/benchmarks/n{n}/benchmark_summary.csv',
            a.root / f'data/benchmarks/n{n}/benchmark_summary.csv',
            a.root / f'results/benchmarks/benchmark_summary_n{n}.csv',
        ]
        path = next((c for c in candidates if c.exists()), None)
        if not path:
            continue
        for row in rows(path):
            if row.get('gamma') == '2':
                value = float(row['median_seconds'])
                if not math.isfinite(value) or value <= 0:
                    raise ValueError(f'Invalid measured timing in {path}')
                measurements.append(row)

    if measurements:
        fig, ax = plt.subplots(figsize=(8, 4.8))
        for method, marker in [('manual_ijk', 'o'), ('manual_ikj', 's'), ('cblas_dgemm', '^')]:
            selected = sorted((r for r in measurements if r['method'] == method), key=lambda r: int(r['n']))
            if selected:
                ax.plot([int(r['n']) for r in selected], [float(r['median_seconds']) for r in selected],
                        marker + '-', label=method)
        ax.set(
            xscale='log',
            yscale='log',
            xlabel='Square dimension n',
            ylabel='Median elapsed wall time (seconds)',
            title='Recorded local timings, gamma = 2'
        )
        ax.grid(True, alpha=0.25)
        ax.legend()
        save(fig, a.output / 'benchmark_times')
        print(f'Saved benchmark_times to {a.output}')

    # precision history comparison plots
    precision_candidates = [
        a.root / 'results/precision/precision_history.csv',
        a.root / 'data/precision/precision_history.csv',
        a.root / 'data/precision_history.csv',
    ]
    prec_path = next((c for c in precision_candidates if c.exists()), None)
    if prec_path:
        data = rows(prec_path)
        for g in ('2', '3'):
            fig, ax = plt.subplots(figsize=(8, 4.8))
            for policy, marker in [('float_storage_float_accum', 'o'),
                                   ('float_storage_double_accum_float_output', 's')]:
                selected = [r for r in data if r['gamma'] == g and r['arithmetic_policy'] == policy]
                values = [float(r['vector_rel_diff']) for r in selected]
                if any(not math.isfinite(v) or v < 0 for v in values):
                    raise ValueError('Invalid discrepancy')
                label = 'Full float' if 'float_accum' in policy else 'Mixed accumulation'
                ax.plot([int(r['iteration']) for r in selected], values, marker + '-', markersize=3, label=label)
            ax.set(
                xlabel='Multiplication index k',
                ylabel='Relative vector discrepancy against double',
                title=f'Precision comparison, gamma = {g}; not an exact error'
            )
            ax.grid(True, alpha=0.25)
            ax.legend()
            save(fig, a.output / f'precision_gamma{g}')
            print(f'Saved precision_gamma{g} to {a.output}')

    return 0

if __name__ == '__main__':
    raise SystemExit(main())

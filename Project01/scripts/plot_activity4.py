#!/usr/bin/env python3
"""Plot existing validated compiler flag benchmark CSVs. Never executes a solver."""

from __future__ import annotations

import argparse
import csv
import math
import pathlib

import matplotlib

matplotlib.use('Agg')
import matplotlib.pyplot as plt


def read_compiler_flags(path: pathlib.Path):
    if not path.exists():
        raise FileNotFoundError(f'Missing benchmark data: {path}')

    with path.open() as f:
        reader = csv.DictReader(f)
        data = list(reader)

    if not data:
        raise ValueError(f'{path}: benchmark data is empty')

    results = []
    baseline_manual = None

    for row in data:
        flag = row['flag']
        man_med = float(row['manual_median_ms'])
        man_spd = float(row['manual_spread_ms'])
        blas_med = float(row['blas_median_ms'])
        blas_spd = float(row['blas_spread_ms'])
        frob_diff = float(row['frobenius_diff'])
        tol = float(row['tolerance'])
        status = row.get('status', 'PASS')

        if flag == '-O0' and baseline_manual is None:
            baseline_manual = man_med

        results.append({
            'flag': flag,
            'manual_median_ms': man_med,
            'manual_spread_ms': man_spd,
            'blas_median_ms': blas_med,
            'blas_spread_ms': blas_spd,
            'frobenius_diff': frob_diff,
            'tolerance': tol,
            'status': status,
        })

    for r in results:
        if baseline_manual and r['manual_median_ms'] > 0:
            r['speedup_vs_O0'] = baseline_manual / r['manual_median_ms']
        else:
            r['speedup_vs_O0'] = 1.0

    return results


def print_table(results):
    print('\n' + '=' * 88)
    print(f'{"Flag":<8} {"Manual (ms)":<14} {"Spread (ms)":<14} {"Speedup":<10} {"BLAS (ms)":<12} {"Frob Diff":<14} {"Status"}')
    print('-' * 88)
    for r in results:
        speedup_str = f'{r["speedup_vs_O0"]:.2f}x'
        print(
            f'{r["flag"]:<8} '
            f'{r["manual_median_ms"]:<14.3f} '
            f'{r["manual_spread_ms"]:<14.3f} '
            f'{speedup_str:<10} '
            f'{r["blas_median_ms"]:<12.3f} '
            f'{r["frobenius_diff"]:<14.2e} '
            f'{r["status"]}'
        )
    print('=' * 88 + '\n')


def generate_plots(results, plots_dir: pathlib.Path):
    plots_dir.mkdir(parents=True, exist_ok=True)

    flags = [r['flag'] for r in results]
    manual_times = [r['manual_median_ms'] for r in results]
    manual_spreads = [r['manual_spread_ms'] for r in results]
    speedups = [r['speedup_vs_O0'] for r in results]
    blas_times = [r['blas_median_ms'] for r in results]
    blas_median = blas_times[0] if blas_times else 0.0

    # 2 Subplots: Kernel Execution Time & Speedup
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11, 4.8))

    # Left plot: Kernel Execution Time (ms)
    bars1 = ax1.bar(
        flags,
        manual_times,
        yerr=manual_spreads,
        capsize=4,
        color='#1f77b4',
        alpha=0.85,
        edgecolor='black',
        label='Manual kernel (median)',
    )
    ax1.axhline(
        blas_median,
        color='#d62728',
        linestyle='--',
        linewidth=1.8,
        label=f'CBLAS dgemm ({blas_median:.2f} ms)',
    )
    ax1.set_xlabel('Compiler Optimization Flag', fontsize=11)
    ax1.set_ylabel('Execution Time (ms)', fontsize=11)
    ax1.set_title('Matrix Square Kernel Timing ($n=256$)', fontsize=12, pad=10)
    ax1.grid(True, axis='y', alpha=0.3)
    ax1.legend(frameon=True, loc='upper right')

    for bar in bars1:
        height = bar.get_height()
        ax1.annotate(
            f'{height:.1f} ms',
            xy=(bar.get_x() + bar.get_width() / 2, height),
            xytext=(0, 5),
            textcoords='offset points',
            ha='center',
            va='bottom',
            fontsize=9,
        )

    # Right plot: Speedup relative to -O0
    bars2 = ax2.bar(
        flags,
        speedups,
        color='#2ca02c',
        alpha=0.85,
        edgecolor='black',
        label='Speedup vs -O0',
    )
    ax2.set_xlabel('Compiler Optimization Flag', fontsize=11)
    ax2.set_ylabel('Speedup Factor ($T_{-O0} / T_{flag}$)', fontsize=11)
    ax2.set_title('Speedup Relative to -O0', fontsize=12, pad=10)
    ax2.grid(True, axis='y', alpha=0.3)
    ax2.set_ylim(0, max(speedups) * 1.25)

    for bar in bars2:
        height = bar.get_height()
        ax2.annotate(
            f'{height:.2f}x',
            xy=(bar.get_x() + bar.get_width() / 2, height),
            xytext=(0, 5),
            textcoords='offset points',
            ha='center',
            va='bottom',
            fontsize=9,
            fontweight='bold',
        )

    fig.tight_layout()
    for ext in ('pdf', 'svg', 'png'):
        fig.savefig(plots_dir / f'compiler_flags.{ext}', dpi=180)
        fig.savefig(plots_dir / f'compiler_flags_timing.{ext}', dpi=180)
    plt.close(fig)

    print(f'Saved compiler flag study plots to {plots_dir}')


def main():
    parser = argparse.ArgumentParser(description='Plot compiler flag benchmark data')
    parser.add_argument('input', type=pathlib.Path, nargs='?', default=pathlib.Path('data'), help='Input directory')
    parser.add_argument('--output', type=pathlib.Path, default=pathlib.Path('plots'), help='Plots output directory')
    args = parser.parse_args()

    csv_path = args.input / 'compiler_flags.csv'
    results = read_compiler_flags(csv_path)

    print_table(results)
    generate_plots(results, args.output)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())

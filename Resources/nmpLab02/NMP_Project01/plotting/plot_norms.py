#!/usr/bin/env python3
"""Plot existing validated post-step norm CSVs. Never executes a solver."""
from __future__ import annotations
import argparse, csv, math, pathlib
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

def read_norms(path:pathlib.Path):
    with path.open() as f:
        r=csv.DictReader(f)
        if r.fieldnames!=['iteration','norm2']:raise ValueError(f'{path}: expected iteration,norm2')
        data=list(r)
    k=[int(r['iteration']) for r in data];y=[float(r['norm2']) for r in data]
    if k!=list(range(1,len(k)+1)) or not k:raise ValueError('Iterations must be contiguous positive integers')
    if any(not math.isfinite(v) or v<=0 for v in y):raise ValueError('Expected finite positive norms')
    return k,y

def main():
    p=argparse.ArgumentParser();p.add_argument('input',type=pathlib.Path);p.add_argument('--output',type=pathlib.Path,default=pathlib.Path('figures'));a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True)
    for g in (2,3):
        x,y=read_norms(a.input/f'norms_gamma{g}.csv')
        fig,ax=plt.subplots(figsize=(8,4.8));ax.plot(x,y,'o-',markersize=3.5,linewidth=1.5)
        ax.set(xlabel='Multiplication index k',ylabel=r'$\|x^{(k)}\|_2$ (dimensionless)',title=f'Gamma = {g}: unnormalized matrix-vector iteration',xlim=(1,max(x)))
        if g==2:ax.set_yscale('log');ax.set_ylabel(r'$\|x^{(k)}\|_2$ (dimensionless, logarithmic axis)')
        else:ax.set_ylim(0,max(y)*1.08)
        ax.grid(True,alpha=.25);fig.tight_layout()
        for ext in ('pdf','svg','png'):fig.savefig(a.output/f'norms_gamma{g}.{ext}',dpi=180)
        plt.close(fig)
    print('Saved separate gamma plots from CSV data.');return 0
if __name__=='__main__':raise SystemExit(main())

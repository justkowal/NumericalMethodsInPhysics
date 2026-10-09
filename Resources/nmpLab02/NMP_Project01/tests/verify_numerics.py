#!/usr/bin/env python3
"""Independent numerical checks: diagonal Toeplitz traversal and Decimal oracles.
No numerical outputs are taken from a previous package. Requires NumPy for large
array operations; Decimal is in Python's standard library. Python is validation,
not the assigned C++ implementation.
"""
from __future__ import annotations
import os
os.environ.setdefault('OPENBLAS_NUM_THREADS','1')
import csv, decimal, hashlib, json, math, pathlib, subprocess
import numpy as np
ROOT=pathlib.Path(__file__).resolve().parents[1]
OUT=ROOT/'results/validation';OUT.mkdir(parents=True,exist_ok=True)
def require(ok: bool, message: str) -> None:
    if not ok: raise RuntimeError(message)
def read_vector(path:pathlib.Path) -> np.ndarray:
    with path.open() as f:
        r=csv.DictReader(f);require(r.fieldnames==['index','value'],f'Bad vector schema {path}');rows=list(r)
    require([int(r['index']) for r in rows]==list(range(len(rows))),'Bad indices')
    x=np.array([float(r['value']) for r in rows]);require(bool(np.all(np.isfinite(x))),'Nonfinite vector');return x

def discrepancy(x,ref):
    delta=np.asarray(x)-np.asarray(ref);scale=math.hypot(*ref);absolute=math.hypot(*delta)
    return dict(max_absolute=float(np.max(np.abs(delta))) if len(delta) else 0.,absolute=absolute,
                reference_norm=scale,relative=absolute/scale if scale else None)

def decimal_history(n:int,g:int,precision:int,stored:bool=False):
    D=decimal.Decimal
    with decimal.localcontext() as c:
        c.prec=precision
        matrix=[[D.from_float(1.0/(float(i-j)**4+g)) if stored else D(1)/(D(i-j)**4+D(g)) for j in range(n)] for i in range(n)]
        x=[D(1)]*n;history=[]
        for _ in range(30):
            x=[sum((matrix[i][j]*x[j] for j in range(n)),D(0)) for i in range(n)];history.append(x)
        return history

def main():
    report={'machine':'local authoring container; not Taurus/classroom','V14':[],'V15':[],'tolerances':{'large_atol':1e-12,'large_rtol':1e-11,'small_atol':1e-13,'small_rtol':1e-12}}
    for g in (2,3):
        official=ROOT/'results/official';rows=list(csv.DictReader((official/f'norms_gamma{g}.csv').open()))
        require(len(rows)==30 and [int(r['iteration']) for r in rows]==list(range(1,31)),'Not thirty post-step rows')
        n=2000;coeff=1/(np.arange(n,dtype=np.float64)**4+float(g));x=np.ones(n);checks=[]
        for k in range(1,31):
            # Independent diagonal-offset traversal, not the C++ row dot loop.
            y=coeff[0]*x
            for distance in range(1,n):
                y[distance:] += coeff[distance]*x[:-distance]
                y[:-distance] += coeff[distance]*x[distance:]
            x=y;candidate=read_vector(official/f'step_gamma{g}_{k}.csv');d=discrepancy(candidate,x)
            require(d['absolute']<=1e-12+1e-11*d['reference_norm'],f'Large recurrence mismatch gamma{g}, step{k}')
            require(abs(float(rows[k-1]['norm2'])-math.hypot(*candidate))<=1e-12+1e-12*math.hypot(*candidate),'Norm implementation mismatch')
            checks.append(dict(iteration=k,**d))
        final=read_vector(official/f'final_vector_gamma{g}.csv');require(np.array_equal(final,candidate),'Final snapshot mismatch')
        norms=[float(r['norm2']) for r in rows];require(all(math.isfinite(v) for v in norms),'Nonfinite norm')
        previous=math.sqrt(n)
        for v in norms:
            require((v>previous) if g==2 else (v<previous),'Unexpected official-size trend');previous=v
        report['V14'].append(dict(gamma=g,n=n,steps=30,full_vectors_compared=30,checks=checks,
                                  final_norm=norms[-1],max_relative=max(d['relative'] for d in checks),status='PASS'))
    for n in (8,16):
        cpp=OUT/f'cpp_n{n}'
        cmd=['./bin/04_gamma_iterations','--n',str(n),'--steps','30','--gamma','both','--snapshots','--output',str(cpp.relative_to(ROOT))]
        if (cpp/'run_metadata.json').exists():cmd+=['--overwrite']
        subprocess.run(cmd,cwd=ROOT,check=True,capture_output=True,text=True)
        for g in (2,3):
            h80=decimal_history(n,g,80);h120=decimal_history(n,g,120);stored=decimal_history(n,g,120,True)
            with decimal.localcontext() as c:
                c.prec=120
                stability=max(abs(a-b)/(1+abs(b)) for va,vb in zip(h80,h120) for a,b in zip(va,vb))
                require(stability<decimal.Decimal('1e-70'),'High-precision reference not stable')
                representation=max(abs(a-b) for a,b in zip(stored[-1],h120[-1]))
            checks=[]
            for k in range(1,31):
                candidate=read_vector(cpp/f'step_gamma{g}_{k}.csv');ref=np.array([float(v) for v in h120[k-1]])
                d=discrepancy(candidate,ref);require(d['absolute']<=1e-13+1e-12*d['reference_norm'],'Small exact-matrix reference failed');checks.append(dict(iteration=k,**d))
            with (OUT/f'decimal_n{n}_gamma{g}.csv').open('w') as f:
                f.write('index,exact_matrix_120_digit_reference,stored_binary64_matrix_120_digit_reference\n')
                for i,(exact,binary) in enumerate(zip(h120[-1],stored[-1])):f.write(f'{i},{exact},{binary}\n')
            report['V15'].append(dict(n=n,gamma=g,steps=30,reference='independent Decimal exact rational matrix; separate exact-binary64-stored matrix',
                precision_comparison='80 versus 120 decimal digits',scaled_reference_change=str(stability),
                representation_max_component_at_step30=str(representation),checks=checks,status='PASS'))
    report['bounds']={'gamma2_rayleigh_lower_n2000':.5+2*(2000-1)/(3*2000),
        'gamma3_row_sum_upper':1/3+1/2+2/19+4/81,'classification':'analytical bounds, NOT eigenvalue estimates'}
    (OUT/'independent_validation.json').write_text(json.dumps(report,indent=2)+'\n')
    for x in report['V14']:print('V14 PASS gamma',x['gamma'],'all 30 full vectors; maximum relative discrepancy',x['max_relative'])
    for x in report['V15']:print('V15 PASS n',x['n'],'gamma',x['gamma'],'80/120 precision change',x['scaled_reference_change'])
    return 0
if __name__=='__main__':raise SystemExit(main())

#!/usr/bin/env python3
"""Optional moderate-size flag study; each output checked in a strict Python process.
Flags on this executable do not change a separately installed BLAS library.
"""
from __future__ import annotations
import csv, hashlib, json, math, pathlib, subprocess
ROOT=pathlib.Path(__file__).resolve().parents[1]
OUT=ROOT/'results/benchmarks/compiler_flags';OUT.mkdir(parents=True,exist_ok=True)

def main():
    n=128;g=2;reference=[]
    # Python math.fsum independently sums binary64 products, no BLAS call.
    for i in range(n):
        for j in range(n):
            reference.append(math.fsum((1/((i-k)**4+g))*(1/((k-j)**4+g)) for k in range(n)))
    report=[]
    for flag in ('-O0','-O1','-O2','-O3','-Ofast'):
        build=ROOT/'bin'/flag[1:];build.mkdir(parents=True,exist_ok=True);exe=build/'08_benchmark'
        command=['g++','-std=c++17','-Wall','-Wextra','-Wpedantic',flag,'-Iinclude','src/08_benchmark.cpp','-o',str(exe.relative_to(ROOT))]
        compiled=subprocess.run(command,cwd=ROOT,capture_output=True,text=True)
        if compiled.returncode:
            report.append(dict(flag=flag,status='UNSUPPORTED/BUILD FAILED',output=compiled.stderr));continue
        dest=OUT/flag[1:]
        cmd=[str(exe),'--n',str(n),'--gamma','2','--repeats','5','--method','manual','--dump-final','--output',str(dest)]
        if (dest/'benchmark_raw.csv').exists():cmd.append('--overwrite')
        r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,check=True)
        rows=list(csv.DictReader((dest/'final_manual_ijk_gamma2.csv').open()))
        if len(rows)!=n*n:raise RuntimeError('Flag matrix row count')
        values=[]
        for index,row in enumerate(rows):
            if (int(row['row']),int(row['column']))!=divmod(index,n):raise RuntimeError('Flag matrix shape/order')
            value=float(row['value'])
            if not math.isfinite(value):raise RuntimeError('Nonfinite fast-math output rejected by independent validator')
            values.append(value)
        differences=[a-b for a,b in zip(values,reference)];absolute=math.hypot(*differences);scale=math.hypot(*reference)
        if absolute>1e-13+1e-12*scale:raise RuntimeError(f'Independent validation failed for {flag}')
        summary=list(csv.DictReader((dest/'benchmark_summary.csv').open()))[0]
        report.append(dict(flag=flag,status='PASS',command=command,run_command=cmd,compiler_stderr=compiled.stderr,
                           binary_sha256=hashlib.sha256(exe.read_bytes()).hexdigest(),median_seconds=float(summary['median_seconds']),
                           max_absolute=max(map(abs,differences)),absolute=absolute,relative=absolute/scale,
                           validator='separate Python process: finite checks + full 128x128 math.fsum reference; atol1e-13 rtol1e-12',stdout=r.stdout))
    (OUT/'strict_validation.json').write_text(json.dumps(report,indent=2)+'\n')
    with (OUT/'flag_summary.csv').open('w',newline='') as f:
        writer=csv.DictWriter(f,fieldnames=['flag','median_seconds','max_absolute','absolute','relative','status']);writer.writeheader()
        for row in report:
            writer.writerow({k:row.get(k,'') for k in writer.fieldnames})
    print('V16/V17 flag study:',[(r['flag'],r['status'],r.get('median_seconds')) for r in report]);return 0
if __name__=='__main__':raise SystemExit(main())

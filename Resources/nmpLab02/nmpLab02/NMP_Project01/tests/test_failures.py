#!/usr/bin/env python3
"""CLI/file safety and dependency-gating tests; no privileged operations."""
import json, pathlib, subprocess, tempfile, os
ROOT=pathlib.Path(__file__).resolve().parents[1]
checks=[]
cases=[(['./bin/04_gamma_iterations','--n',v],2) for v in ['0','-1','2x','9999999999999999999999999999999999999']]
cases += [(['./bin/04_gamma_iterations','--gamma','4'],2),(['./bin/04_gamma_iterations','--steps','-1'],2),
          (['./bin/04_gamma_iterations','--unknown','3'],2),(['./bin/04_gamma_iterations','--n','5000'],2),
          (['./bin/08_benchmark','--n','2000'],2),(['./bin/08_benchmark','--method','blas'],2),
          (['./bin/04_gamma_iterations','--n','2','--output','/proc/nmp_project01_no_write'],2),
          (['./bin/04_gamma_iterations','--output','results/official'],2)]
for cmd,expected in cases:
    r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True)
    if r.returncode!=expected:raise RuntimeError(f'{cmd}: expected {expected}, got {r.returncode}')
    checks.append(dict(command=cmd,expected=expected,returncode=r.returncode,output=r.stdout+r.stderr,status='PASS'))
with tempfile.TemporaryDirectory() as temp:
    t=pathlib.Path(temp);(t/'bad.csv').write_text('index,value\n0,nan\n');(t/'ref.csv').write_text('index,value\n0,0\n')
    cmd=['./bin/07_validate_compare','--candidate',str(t/'bad.csv'),'--reference',str(t/'ref.csv')]
    r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True)
    if r.returncode!=2:raise RuntimeError('Strict validator accepted nonfinite input')
    checks.append(dict(command='strict validator with temporary NaN fixture',returncode=r.returncode,output=r.stderr,status='PASS'))
env=dict(os.environ,BLAS_LIBS='-lNMP_INTENTIONALLY_MISSING_LIBRARY')
r=subprocess.run(['bash','scripts/check_env.sh'],cwd=ROOT,capture_output=True,text=True,env=env)
if r.returncode!=3:raise RuntimeError('Missing BLAS was not rejected by probe')
checks.append(dict(command='BLAS_LIBS=-lNMP_INTENTIONALLY_MISSING_LIBRARY bash scripts/check_env.sh',returncode=r.returncode,status='PASS: expected dependency failure',output=r.stdout+r.stderr))
(ROOT/'results/validation/cli_failure_tests.json').write_text(json.dumps(checks,indent=2)+'\n')
print(f'V13/V17 PASS: {len(checks)} controlled failure cases, including strict NaN and missing BLAS.')

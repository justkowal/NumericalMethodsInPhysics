#!/usr/bin/env python3
"""Run a local command, retain output, hashes and observed process thread count.
This is evidence for the machine running it, not certification of another host.
"""
from __future__ import annotations
import argparse, datetime, hashlib, json, os, pathlib, subprocess, time

def main() -> int:
    p=argparse.ArgumentParser();p.add_argument('--id',required=True);p.add_argument('--timeout',type=float,default=180)
    p.add_argument('command',nargs=argparse.REMAINDER);a=p.parse_args()
    command=a.command[1:] if a.command and a.command[0]=='--' else a.command
    if not command:p.error('a command is required after --')
    root=pathlib.Path(__file__).resolve().parents[1]; logs=root/'verification/logs';logs.mkdir(parents=True,exist_ok=True)
    log=logs/(a.id+'.log');start=time.monotonic();date=datetime.datetime.now(datetime.timezone.utc).isoformat()
    max_threads=0;timed_out=False
    with log.open('w') as f:
        f.write('$ '+' '.join(command)+'\n');f.flush()
        process=subprocess.Popen(command,cwd=root,stdout=f,stderr=subprocess.STDOUT)
        while process.poll() is None:
            try:
                fields=pathlib.Path(f'/proc/{process.pid}/status').read_text().splitlines()
                max_threads=max(max_threads,*[int(s.split()[1]) for s in fields if s.startswith('Threads:')])
            except (OSError,ValueError):pass
            if time.monotonic()-start>a.timeout:
                timed_out=True;process.kill();process.wait();break
            time.sleep(.01)
    elapsed=time.monotonic()-start;index=logs/'recorded_runs.json'
    records=json.loads(index.read_text()) if index.exists() else []
    exe=(root/command[0]).resolve()
    records.append(dict(run_id=a.id,command=command,date_utc=date,returncode=process.returncode,
                        timed_out=timed_out,elapsed_seconds=elapsed,observed_max_process_threads=max_threads or None,
                        log=log.relative_to(root).as_posix(),
                        executable_sha256=hashlib.sha256(exe.read_bytes()).hexdigest() if exe.is_file() else None,
                        machine_category='local isolated authoring container; NOT Taurus/classroom'))
    index.write_text(json.dumps(records,indent=2)+'\n')
    print(log.read_text(),end='');print(f'[recorded {a.id}: exit={process.returncode}; elapsed={elapsed:.3f}s; max observed threads={max_threads}]')
    return 124 if timed_out else process.returncode
if __name__=='__main__':raise SystemExit(main())

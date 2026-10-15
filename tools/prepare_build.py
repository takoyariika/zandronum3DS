"""Prepare a source checkout without personal games or reference repositories."""
import argparse
import shutil
import subprocess
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--re2c',default='re2c',help='Path to the host re2c executable')
p.add_argument('--makerom',type=Path,help='Optional makerom executable to copy locally for CIA packaging')
a=p.parse_args()
out=ROOT/'build-3ds'; out.mkdir(exist_ok=True)
subprocess.run([a.re2c,'--no-generation-date','-s','-o',str(out/'sc_man_scanner.h'),str(ROOT/'upstream/src/sc_man_scanner.re')],check=True)
if a.makerom:
    dest=ROOT/'dependencies/makerom-0.18.3/makerom.exe'; dest.parent.mkdir(parents=True,exist_ok=True)
    if a.makerom.resolve()!=dest.resolve(): shutil.copy2(a.makerom,dest)
print('Scanner ready. Dependency sources, XLAT parser and CIA mappings are included.')
print('No original game IWAD or MOD is required to build the application.')

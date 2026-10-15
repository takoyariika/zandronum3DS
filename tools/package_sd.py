"""Stage engine data and the user's own IWAD/mod in a workspace-local SD tree."""
import argparse
from pathlib import Path
import shutil
import zipfile
import json
import hashlib
ROOT = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--iwad', type=Path, help='Optional personal IWAD, for local testing only')
p.add_argument('--byoc', type=Path)
a = p.parse_args()
if a.iwad:
    with a.iwad.open('rb') as f:
        if f.read(4) != b'IWAD': raise ValueError('Expected an IWAD')
dest = ROOT/'build-3ds/sdmc/3ds/zandronum'
dest.mkdir(parents=True, exist_ok=True)
manifest=json.loads((ROOT/'reference-generated/engine-resource-manifest.json').read_text())
for name, directory in [('zandronum.pk3', 'wadsrc'), ('skulltag_actors.pk3', 'wadsrc_st')]:
    source = ROOT/'upstream'/directory/'static'
    with zipfile.ZipFile(dest/name, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=6) as z:
        for file in sorted(source.rglob('*')):
            if file.is_file():
                data=file.read_bytes()
                # The official archive intentionally mixes LF and CRLF. Undo
                # only checkout conversion, using canonical per-resource hashes.
                expected=manifest[file.relative_to(ROOT/'upstream').as_posix()]
                if hashlib.sha256(data).hexdigest()!=expected:
                    data=data.replace(b'\r\n',b'\n')
                if hashlib.sha256(data).hexdigest()!=expected:
                    raise ValueError('Engine resource differs from pinned release: '+str(file))
                z.writestr(file.relative_to(source).as_posix(),data)
if a.iwad: shutil.copy2(a.iwad, dest/'DOOM2.WAD')
if a.byoc:
    with zipfile.ZipFile(a.byoc) as z:
        if not z.namelist(): raise ValueError('Empty mod')
    shutil.copy2(a.byoc, dest/'BYOCv2.0.pk3')
(dest/'args-doom.txt').write_text('\n')
(dest/'args-byoc.txt').write_text('-file sdmc:/3ds/zandronum/BYOCv2.0.pk3 -nosound\n')
shutil.copy2(dest/'args-doom.txt', dest/'args.txt')
print(dest)
print('Only engine assets staged.' if not a.iwad and not a.byoc else 'Contains personal game/MOD data: do not redistribute the SD tree.')

"""Extract pinned, already downloaded dependency archives inside this workspace."""
import hashlib
import json
import pathlib
import tarfile
import zipfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
OUT = ROOT/'dependencies'
OUT.mkdir(exist_ok=True)
records = []
expected = {
    'sqlite.zip': '999826fe4c871f18919fdb8ed7ec9dd8217180854dd1fe21eea96aed36186729',
    'zstd.tar.gz': 'eb33e51f49a15e023950cd7825ca74a4a2b43db8354825ac24fc1b7ee09e6fa3'
}
for name in ('sqlite.zip', 'zstd.tar.gz'):
    path = ROOT/'build-3ds'/name
    digest = hashlib.sha256(path.read_bytes()).hexdigest()
    if digest != expected[name]: raise ValueError('Dependency archive hash mismatch: '+name)
    records.append({'file': name, 'sha256': digest})
    if name.endswith('.zip'):
        with zipfile.ZipFile(path) as archive:
            for member in archive.infolist():
                target = (OUT/member.filename).resolve()
                if not target.is_relative_to(OUT.resolve()):
                    raise ValueError('Unsafe ZIP path')
                archive.extract(member, OUT)
    else:
        with tarfile.open(path) as archive:
            for member in archive.getmembers():
                target = (OUT/member.name).resolve()
                if not target.is_relative_to(OUT.resolve()):
                    raise ValueError('Unsafe TAR path')
                if member.issym() or member.islnk():
                    continue # documentation/program links are not needed by the library build
                if member.isdir():
                    target.mkdir(parents=True, exist_ok=True)
                elif member.isfile():
                    target.parent.mkdir(parents=True, exist_ok=True)
                    with archive.extractfile(member) as inp, target.open('wb') as dest:
                        import shutil
                        shutil.copyfileobj(inp, dest)
                else:
                    raise ValueError('Unsupported TAR member type')
(OUT/'archives.lock.json').write_text(json.dumps(records, indent=2))
print('Extracted dependencies into', OUT)

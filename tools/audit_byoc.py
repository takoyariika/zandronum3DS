"""Read a local BYOC PK3 without extracting or modifying it."""
import argparse
import collections
import hashlib
import json
import pathlib
import re
import zipfile

def main():
    p = argparse.ArgumentParser()
    p.add_argument('pk3', type=pathlib.Path)
    p.add_argument('--output', type=pathlib.Path, default=pathlib.Path('build-3ds/byoc-report.json'))
    a = p.parse_args()
    digest = hashlib.sha256()
    with a.pk3.open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            digest.update(chunk)
    with zipfile.ZipFile(a.pk3) as z:
        entries = [e for e in z.infolist() if not e.is_dir()]
        scripts = []
        actions = collections.Counter()
        actors = 0
        for e in entries:
            path = e.filename.lower()
            if path.startswith(('decorate/', 'actors/')) or path.endswith(('.txt', '.dec', '.decorate', '.acs')) or pathlib.PurePosixPath(path).name.split('.')[0] in ('decorate', 'sbarinfo', 'mapinfo', 'keyconf', 'loadacs', 'sndinfo', 'menudef', 'zscript'):
                t = z.read(e).decode('latin1')
                scripts.append({'path': e.filename, 'bytes': e.file_size})
                if 'decorate' in path or re.search(r'\bactor\s+\w+', t, re.I):
                    actors += len(re.findall(r'\bactor\s+\w+', t, re.I))
                    actions.update(re.findall(r'\b(A_\w+)\s*\(', t, re.I))
        r = {'file': str(a.pk3), 'sha256': digest.hexdigest(), 'compressed_bytes': a.pk3.stat().st_size,
             'uncompressed_bytes': sum(e.file_size for e in entries), 'entries': len(entries),
             'compression_methods': dict(collections.Counter(e.compress_type for e in entries)),
             'extension_bytes': dict(collections.Counter({ext: sum(e.file_size for e in entries if pathlib.PurePosixPath(e.filename).suffix.lower() == ext) for ext in sorted({pathlib.PurePosixPath(e.filename).suffix.lower() for e in entries})})),
             'largest_entries': [{'path': e.filename, 'bytes': e.file_size} for e in sorted(entries, key=lambda e: e.file_size, reverse=True)[:20]],
             'actor_declarations': actors, 'decorate_action_calls': dict(actions.most_common()), 'scripts': scripts,
             'note': 'Archive size is not peak RAM. Runtime parsing, decoded textures, sound caches and map state also allocate memory.'}
    a.output.parent.mkdir(parents=True, exist_ok=True)
    a.output.write_text(json.dumps(r, indent=2), encoding='utf-8')
    print(json.dumps({k: r[k] for k in ('compressed_bytes', 'uncompressed_bytes', 'entries', 'actor_declarations', 'compression_methods')}, indent=2))

if __name__ == '__main__':
    main()

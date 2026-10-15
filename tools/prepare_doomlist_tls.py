"""Extract the pinned official Mbed TLS release used by the 3DS server browser."""
from pathlib import Path
import hashlib, tarfile
root=Path(__file__).resolve().parents[1]
archive=root/'build-3ds/mbedtls-3.6.7.tar.bz2'
assert hashlib.sha256(archive.read_bytes()).hexdigest()=='a7e8bcbec0e6f761b4af24f25677626b35f762f68eef79c08677a363212d11f6'
dest=(root/'dependencies').resolve()
with tarfile.open(archive) as tar:
    for member in tar:
        target=(dest/member.name).resolve()
        if not target.is_relative_to(dest): raise ValueError('Unsafe archive path')
        if member.isfile():
            target.parent.mkdir(parents=True,exist_ok=True)
            with tar.extractfile(member) as source, target.open('wb') as output:
                output.write(source.read())
print('Extracted Mbed TLS 3.6.7')

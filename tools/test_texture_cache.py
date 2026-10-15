"""Exercise the shipped PowerShell tool with stored/deflated PNGs and fallbacks."""
import hashlib
import struct
import subprocess
import zipfile
import zlib
from pathlib import Path
from verify_texture_cache import verify

root = Path(__file__).resolve().parents[1]
folder = root/'build-3ds/cache-test'
folder.mkdir(exist_ok=True)
def chunk(tag, data):
    return struct.pack('>I',len(data))+tag+data+struct.pack('>I',zlib.crc32(tag+data))
def png(depth=8, extra=b''):
    return (b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',1,1,depth,6,0,0,0))+
            chunk(b'grAb',struct.pack('>ii',-7,12))+extra+
            chunk(b'IDAT',zlib.compress(b'\x00\xff\x00\x00\xff'))+chunk(b'IEND',b''))
archive=folder/'mixed.pk3'
with zipfile.ZipFile(archive,'w') as z:
    z.writestr('sprites/store.png',png(),compress_type=zipfile.ZIP_STORED)
    z.writestr('sprites/deflate.png',png(),compress_type=zipfile.ZIP_DEFLATED)
    z.writestr('scripts/not-png.txt','hello',compress_type=zipfile.ZIP_DEFLATED)
    z.writestr('sprites/16bit.png',png(16))
    z.writestr('sprites/large-metadata.png',png(extra=chunk(b'tEXt',b'x'*70000)))
# ZIP central-directory order is independent of physical entry offsets.
# Exercise PowerShell 5.1 sorting with a deliberately reversed directory.
raw=archive.read_bytes(); end=raw.rfind(b'PK\x05\x06')
directory_size,directory_offset=struct.unpack_from('<II',raw,end+12)
records=[];pos=directory_offset
while pos<directory_offset+directory_size:
    name,extra,comment=struct.unpack_from('<HHH',raw,pos+28)
    length=46+name+extra+comment;records.append(raw[pos:pos+length]);pos+=length
archive.write_bytes(raw[:directory_offset]+b''.join(reversed(records))+raw[directory_offset+directory_size:])
before=hashlib.sha256(archive.read_bytes()).digest()
subprocess.run(['powershell.exe','-NoProfile','-ExecutionPolicy','Bypass','-File',str(root/'tools/Build-TextureCache.ps1'),'-Archive',str(archive)],check=True)
cache=Path(str(archive)+'.3ds-cache')
assert struct.unpack_from('<I',cache.read_bytes(),28)[0]==2
assert hashlib.sha256(archive.read_bytes()).digest()==before
verify(archive,cache)
original=cache.read_bytes()
cache.write_bytes(original[:-1]+bytes([original[-1]^1]))
try:
    verify(archive,cache)
except AssertionError:
    pass
else:
    raise AssertionError('Corrupt cache accepted')
cache.write_bytes(original)
with zipfile.ZipFile(archive,'a') as z:
    z.writestr('updated.txt','changed archive')
try:
    verify(archive,cache)
except AssertionError:
    pass
else:
    raise AssertionError('Stale cache accepted')
print('Stored/deflated PNGs, offsets, source preservation, unsupported metadata, corrupt/stale detection: OK')

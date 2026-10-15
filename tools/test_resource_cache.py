"""Round-trip mixed ZIP resources and WAD duplicate names/empty markers."""
import hashlib, struct, subprocess, zipfile
from pathlib import Path
from verify_resource_cache import verify
root=Path(__file__).resolve().parents[1]
folder=root/'build-3ds/resource-cache-test';folder.mkdir(exist_ok=True)
wad=folder/'mixed.wad'
data=[(b'MARKER',b''),(b'DUP',b'first'),(b'DUP',b'second'),(b'SCRIPT',b'print(1);'),(b'SOUND',bytes(range(256))*64)]
body=b'';table=b''
for name,value in data:
    table+=struct.pack('<II8s',12+len(body),len(value),name);body+=value
wad.write_bytes(b'PWAD'+struct.pack('<II',len(data),12+len(body))+body+table)
pk3=folder/'mixed.pk3'
with zipfile.ZipFile(pk3,'w') as z:
    z.writestr('dir/',b'')
    z.writestr('scripts/source.txt',b'print(1);',compress_type=zipfile.ZIP_DEFLATED)
    z.writestr('sound/raw.bin',bytes(range(256))*1024,compress_type=zipfile.ZIP_STORED)
    z.writestr('empty',b'')
for archive in (wad,pk3):
    original=hashlib.sha256(archive.read_bytes()).digest()
    subprocess.run(['powershell.exe','-NoProfile','-ExecutionPolicy','Bypass','-File',str(root/'tools/Build-ResourceCache.ps1'),'-Archive',str(archive)],check=True)
    assert hashlib.sha256(archive.read_bytes()).digest()==original
    cache=Path(str(archive)+'.3ds-resources');verify(archive,cache)
    raw=bytearray(cache.read_bytes());raw[-1]^=1;cache.write_bytes(raw)
    try: verify(archive,cache)
    except AssertionError: pass
    else: raise AssertionError('Corrupt resource accepted')
print('All-resource WAD/ZIP round trips, duplicate names, empty markers, corruption and source preservation: OK')

# Both shipped entry points must accept both WAD kinds, regardless of suffix.
for magic in (b'IWAD', b'PWAD'):
    source=folder/(magic.decode()+'.renamed')
    source.write_bytes(magic+wad.read_bytes()[4:])
    original=source.read_bytes()
    for tool in ('Build-ResourceCache.ps1','Build-TextureCache.ps1'):
        command=['powershell.exe','-NoProfile','-ExecutionPolicy','Bypass','-File',str(root/'tools'/tool),'-Archive',str(source)]
        subprocess.run(command,check=True)
        verify(source,Path(str(source)+'.3ds-resources'))
        assert not Path(str(source)+'.3ds-cache').exists()
        assert source.read_bytes()==original
    custom=folder/(magic.decode()+'.custom.3ds-resources')
    subprocess.run(command+['-Output',str(custom)],check=True)
    verify(source,custom)
    invalid=folder/(magic.decode()+'.3ds-cache')
    assert subprocess.run(command+['-Output',str(invalid)],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL).returncode!=0
    assert not invalid.exists()
    assert source.read_bytes()==original
print('IWAD/PWAD through both tools, signature detection, custom output and wrong-format rejection: OK')

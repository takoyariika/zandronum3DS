"""Compare every prepared resource with its original WAD/ZIP bytes."""
import hashlib, struct, zipfile, zlib
from pathlib import Path
import argparse

def verify(source, cache):
    original=source.read_bytes()
    with cache.open('rb') as f:
        h=f.read(52)
        assert h[:8]==b'Z3DRC001'
        source_size,kind,count=struct.unpack_from('<III',h,24)
        assert source_size==len(original)
        if kind==2:
            assert original[:4] in (b'IWAD',b'PWAD')
            n,offset=struct.unpack_from('<II',original,4)
            directory=original[offset:offset+n*16]
            entries={i:original[pos:pos+size] for i in range(n) for pos,size in [struct.unpack_from('<II',directory,i*16)]}
        else:
            end=original.rfind(b'PK\x05\x06',max(0,len(original)-65557))
            size,offset=struct.unpack_from('<II',original,end+12)
            directory=original[offset:offset+size]
            z=zipfile.ZipFile(source)
            entries={e.header_offset:e for e in z.infolist()}
        assert hashlib.md5(directory).digest()==h[8:24]
        table=f.read(count*16)
        assert hashlib.md5(table).digest()==h[36:52]
        assert count==len(entries)
        for i in range(count):
            key,size,offset,crc=struct.unpack_from('<IIII',table,i*16)
            f.seek(offset); data=f.read(size)
            assert len(data)==size and zlib.crc32(data)==crc
            expected=entries[key] if kind==2 else z.read(entries[key])
            assert data==expected, key
        if kind!=2: z.close()
    print(f'Verified all {count} resources: {source.name}')

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('source',type=Path);p.add_argument('cache',type=Path)
    a=p.parse_args();verify(a.source,a.cache)

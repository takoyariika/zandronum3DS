"""Verify every prepared PNG prefix against its original ZIP entry."""
import argparse
import hashlib
import struct
import zipfile
from pathlib import Path

def verify(archive, cache):
    raw = archive.read_bytes()
    end = raw.rfind(b'PK\x05\x06', max(0, len(raw)-65557))
    directory_size, directory_offset = struct.unpack_from('<II', raw, end+12)
    directory = raw[directory_offset:directory_offset+directory_size]
    prepared = cache.read_bytes()
    assert prepared[:8] == b'Z3DTH003'
    assert prepared[8:24] == hashlib.md5(directory).digest()
    size, count = struct.unpack_from('<II', prepared, 24)
    assert size == len(raw)
    assert prepared[32:48] == hashlib.md5(prepared[48:]).digest()
    with zipfile.ZipFile(archive) as z:
        entries = {entry.header_offset: entry for entry in z.infolist()}
        pos, previous = 48, -1
        for _ in range(count):
            offset, size, compressed, crc, length = struct.unpack_from('<IIIII', prepared, pos)
            pos += 20
            prefix = prepared[pos:pos+length]
            pos += length
            assert offset > previous
            previous = offset
            entry = entries[offset]
            assert (size, compressed, crc) == (entry.file_size, entry.compress_size, entry.CRC)
            with z.open(entry) as source:
                assert prefix == source.read(length), entry.filename
            assert prefix[:8] == b'\x89PNG\r\n\x1a\n'
            chunk = 8
            while chunk+8 <= len(prefix):
                length = struct.unpack_from('>I', prefix, chunk)[0]
                tag = prefix[chunk+4:chunk+8]
                if tag == b'IDAT':
                    assert chunk+8 == len(prefix)
                    break
                chunk += 12+length
            else:
                raise AssertionError('Missing IDAT')
        assert pos == len(prepared)
    print(f'Verified {count} PNG headers, archive fingerprint and payload checksum.')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('archive', type=Path)
    parser.add_argument('cache', type=Path)
    args = parser.parse_args()
    verify(args.archive, args.cache)

"""Validate observed decoder output and memory plateau in native Citra replay logs."""
import argparse,re
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('log',type=Path);p.add_argument('--stderr',type=Path);p.add_argument('--shotgun',action='store_true')
a=p.parse_args(); text=a.log.read_text(errors='replace')
for codec in ('WAV','Vorbis','MP3','FLAC'):
    assert 'AUDIO DECODER opened '+codec in text,codec+' did not open'
assert 'AUDIO DECODER TEST completed' in text
decoded=re.findall(r'AUDIO DECODED frames=(\d+) bytes=(\d+) rate=(\d+) channels=(\d+)',text)
assert len(decoded)>=4 and all(int(n)>0 for row in decoded for n in row),decoded
stats=[tuple(map(int,row)) for row in re.findall(r'effects=(\d+) stream-nonzero=(\d+) samples=(\d+)/(\d+) KiB',text)]
assert stats and max(row[0] for row in stats)>=4 and max(row[1] for row in stats)>0,stats
assert all(row[2]<=row[3] for row in stats),'Sample budget exceeded'
assert 'SLEEP TEST' in text
if a.shotgun:
    assert 'pawn=DoomGuy' in text
    assert 'SHOTGUN TEST completed: 120 seconds' in text
    heaps=[int(n) for n in re.findall(r'SHOTGUN TEST elapsed=\d+ heap=(\d+)',text)]
    assert len(heaps)>=12,heaps
    assert max(heaps[-6:])-min(heaps[-6:])<4096,heaps
    assert heaps[-1]<heaps[0]+8192,heaps
    assert max(row[0] for row in stats)>=50,'Attack replay did not generate enough sounds'
    print('Shotgun heap KiB: min=',min(heaps),'max=',max(heaps),'last=',heaps[-1])
assert not re.search(r'Out of memory|FATAL ERROR',text,re.I)
if a.stderr:
    errors=a.stderr.read_text(errors='replace')
    assert not re.search(r'unmapped (Read|Write)|Fatal Error|Assertion Failed',errors,re.I)
    assert 'Cleaning up process' in errors,'No natural process exit'
print('Native audio decoders, nonzero PCM, bounded SFX cache and exit verified. Physical audio output is not measured.')

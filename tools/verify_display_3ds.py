"""Check native LCD output, title transition and sustained gameplay logs."""
from pathlib import Path
import argparse,re
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('log',type=Path);p.add_argument('video',type=Path);p.add_argument('stderr',type=Path);p.add_argument('--shotgun',action='store_true')
a=p.parse_args(); text=a.log.read_text(errors='replace'); video=a.video.read_text(errors='replace'); errors=a.stderr.read_text(errors='replace')
assert 'TITLE TEST:' in text and 'title to BYOCTEST' in text
frames=re.findall(r'state=(\d+) canvas=(\d+)x(\d+) pitch=(\d+) LCD=(\d+)x(\d+) nonblack=(\d+)/96000',video)
assert any(int(f[0])==7 and int(f[6])>50000 for f in frames),'No visible title LCD image'
assert any(int(f[0])==0 and int(f[6])>50000 for f in frames),'No visible map LCD image'
assert all(tuple(map(int,f[1:3]))==(400,240) and tuple(map(int,f[4:6]))==(240,400) for f in frames)
assert 'SLEEP TEST: clock delta=0 ms' in text
assert not re.search(r'Out of memory|FATAL ERROR',text,re.I)
assert not re.search(r'unmapped (Read|Write)|Fatal Error|Assertion Failed',errors,re.I)
assert 'Cleaning up process' in errors
if a.shotgun:
    assert 'SHOTGUN TEST completed: 120 seconds' in text
    heaps=[int(v) for v in re.findall(r'SHOTGUN TEST elapsed=\d+ heap=(\d+)',text)]
    assert len(heaps)>=12 and max(heaps[-6:])-min(heaps[-6:])<4096,heaps
    assert heaps[-1]<heaps[0]+8192,heaps
    print('Shotgun heap KiB:',min(heaps),max(heaps),'last',heaps[-1])
print('Native LCD title/map content, sleep callback and normal exit: OK. Physical-console regression remains unverified.')

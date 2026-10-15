"""Validate real frame samples and native input replay, without claiming hardware FPS."""
import argparse
import re
from pathlib import Path

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('performance', type=Path)
p.add_argument('engine', type=Path)
p.add_argument('stderr', type=Path)
p.add_argument('--baseline', type=Path)
a = p.parse_args()
text = a.performance.read_text()
rows = [tuple(map(int, r)) for r in re.findall(
    r'state=(\d+) frames=(\d+) elapsed=(\d+) ms .*?heap=(\d+) KiB gametic=(\d+)', text)]
title = [r for r in rows if r[0] == 7]
game = [r for r in rows if r[0] == 0]
assert len(title) >= 8, 'Need at least 80 seconds of animated title samples'
assert game, 'No game frames after the title'
assert all(b[4] > c[4] for c, b in zip(title, title[1:])), 'Title simulation stopped'
assert max(r[3] for r in title) < 80000, 'Title heap is too close to exhaustion'
assert max(r[3] for r in title[-4:]) - min(r[3] for r in title[-4:]) < 4096, 'Title heap has not stabilized'
fps = sum(r[1] for r in title[2:]) * 1000 / sum(r[2] for r in title[2:])
assert fps >= 8, f'Title is still too slow in the reference emulator: {fps:.2f} fps'
engine = a.engine.read_text(errors='replace')
assert 'title to BYOCTEST' in engine
assert 'INPUT TEST completed' in engine or 'INPUT COMBAT TEST completed' in engine
assert 'normal menu exit path' in engine
assert not re.search(r'Out of memory|FATAL ERROR', engine, re.I)
errors = a.stderr.read_text(errors='replace')
assert not re.search(r'unmapped (Read|Write)|Fatal Error|Assertion Failed', errors, re.I)
assert 'Cleaning up process' in errors
print(f'Animated title: {fps:.2f} fps; heap {min(r[3] for r in title)}..{max(r[3] for r in title)} KiB')
if a.baseline:
    old = [tuple(map(int, r)) for r in re.findall(r'state=7 frames=(\d+) elapsed=(\d+) ms', a.baseline.read_text())][2:]
    assert old
    before = sum(r[0] for r in old) * 1000 / sum(r[1] for r in old)
    assert fps > before * 2, 'Expected a substantial frame-rate improvement'
    print(f'Previous input-time heap scanning: {before:.2f} fps; improvement {fps/before:.2f}x')
print('Title simulation, bounded heap, gameplay input and normal exit: OK. Physical hardware remains unverified.')

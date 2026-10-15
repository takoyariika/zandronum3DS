"""Validate CIA launcher, touch replay, sleep clock and quit regression logs.

The sleep check invokes APT callbacks; it does not emulate a physical lid.
Run run_citra.ps1 with LauncherTest/InputTest/CombatTest/TouchTest/SleepTest/QuitTest.
"""
import re
import sys
from pathlib import Path

startup = Path(sys.argv[1]).read_text(errors='replace')
stderr = Path(sys.argv[2]).read_text(errors='replace')
assert 'DOOM2.WAD' in startup and 'BYOCv2.0.pk3' in startup
assert re.search(r'adding .*Chooser test\.wad.*, 0 lumps', startup), 'Selected PWAD with spaces did not load'
assert startup.index('adding sdmc:/3ds/zandronum/BYOCv2.0.pk3') < startup.index('adding sdmc:/3ds/zandronum/Chooser test.wad')
sleep = re.search(r'SLEEP TEST: clock delta=(\d+) ms, active=1, generation=(\d+)', startup)
assert sleep and int(sleep[1]) <= 5 and int(sleep[2]) > 0
assert 'TOUCH TEST: gain=12.00' in startup
before = re.search(r'INPUT TEST phase=4 .*angle=(\d+) pitch=(-?\d+)', startup)
after = re.search(r'TOUCH VIEW TEST: angle=(\d+) pitch=(-?\d+)', startup)
assert before and after and before[1] != after[1] and before[2] != after[2]
assert 'QUIT TEST: normal menu exit path' in startup
assert not re.search(r'unmapped (Read|Write)|Fatal Error|Assertion Failed', stderr)
assert 'Cleaning up process' in stderr
assert 'FATAL ERROR' not in startup
print('IWAD + ordered PK3/PWAD (spaces), 12x touch yaw/pitch, sleep clock callbacks, clean CIA quit: OK')

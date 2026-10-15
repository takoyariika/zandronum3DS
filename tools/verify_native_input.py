"""Check actual player movement/view changes from the native-HID replay."""
import re,sys
from pathlib import Path
text=Path(sys.argv[1]).read_text(errors='replace')
assert 'up=weapnext, right=+moveright' in text
start=re.search(r'pawn=(\w+) x=(-?\d+) y=(-?\d+)',text)
assert start
phases={int(n):tuple(map(int,(x,y,angle,pitch))) for n,x,y,angle,pitch in
        re.findall(r'INPUT TEST phase=(\d+) x=(-?\d+) y=(-?\d+) angle=(\d+) pitch=(-?\d+)',text)}
assert set(phases)=={1,2,3,4}
assert phases[1][:2]!=tuple(map(int,start.groups()[1:])), 'Circle Pad did not move'
assert phases[2][2]!=phases[1][2] and phases[2][3]!=phases[1][3], 'C-stick did not change yaw/pitch'
assert phases[3][:2]!=phases[2][:2], 'D-pad left did not strafe'
assert phases[4][:2]!=phases[3][:2], 'D-pad right did not strafe'
assert phases[4][2]==phases[3][2], 'D-pad strafing changed yaw'
pad=re.findall(r'PAD YAW TEST (before|after) x=(-?\d+) y=(-?\d+) angle=(\d+)',text)
assert len(pad)==2 and pad[0][3]!=pad[1][3], 'Circle Pad X did not turn'
# Allow up to two map units of residual player momentum while turning.
assert all(abs(int(pad[0][i])-int(pad[1][i]))<2*65536 for i in (1,2)), 'Circle Pad X caused strafing'
if 'CONTROL TEST PASS' in text:
    for button in ('A attack','B jump','Y crouch','X use','L attack','R altattack'):
        assert 'CONTROL TEST PASS: '+button in text
    weapons=re.findall(r'WEAPON TEST: (.+) -> (\w+)',text)
    assert len(weapons)==4 and all(weapons[i][1]!=weapons[i-1][1] for i in range(1,4)), 'Weapon cycling did not change weapons'
touch=re.search(r'TOUCH START TEST: pitch=(-?\d+) invert=(\d+)',text)
view=re.search(r'TOUCH VIEW TEST: angle=\d+ pitch=(-?\d+)',text)
if touch:
    assert view, 'Touch replay did not finish'
    delta=int(view[1])-int(touch[1])
    assert delta>0 if int(touch[2]) else delta<0, 'Upward touch drag had the wrong vertical direction'
if start[1]=='AVGNclass':
    assert 'INPUT COMBAT TEST completed' in text
assert 'FATAL ERROR' not in text
print(f'{start[1]}: Circle Pad forward/yaw, C-stick look, D-pad strafe, controls and touch: OK')

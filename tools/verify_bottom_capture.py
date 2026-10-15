"""Verify native lower-LCD theme/button states and make a three-view preview."""
import argparse
import struct
from pathlib import Path
from PIL import Image

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('prefix',type=Path,help='Capture prefix, e.g. build-3ds/doom-cia')
p.add_argument('output',type=Path)
a=p.parse_args()
images=[]
def rgb565(r,g,b):
    return ((r>>3)<<11)|((g>>2)<<5)|(b>>3)
for name in ('walk','scores','log'):
    data=Path(str(a.prefix)+'-ui-bottom-'+name+'-bottom.rgb565').read_bytes()
    assert struct.unpack_from('<HH',data)==(320,240)
    assert len(data)==153604
    colors=struct.unpack_from('<76800H',data,4)
    def pixel(x,y): return colors[x*240+239-y]
    assert pixel(0,0)==rgb565(13,19,29), 'Menu background mismatch'
    assert pixel(8,42)==rgb565(65,219,178), 'Accent missing'
    assert pixel(170,8)==rgb565(*( (34,68,85) if name=='walk' else (24,34,49) )), 'Walk highlight mismatch'
    assert pixel(10,8)==rgb565(*( (34,68,85) if name=='scores' else (24,34,49) )), 'Score toggle highlight mismatch'
    im=Image.new('RGB',(320,240))
    im.putdata([((c>>11)*255//31,((c>>5)&63)*255//63,(c&31)*255//31)
                for y in range(240) for x in range(320) for c in [pixel(x,y)]])
    images.append(im)
canvas=Image.new('RGB',(960,240))
for i,im in enumerate(images): canvas.paste(im,(i*320,0))
canvas.resize((1920,480),Image.Resampling.NEAREST).save(a.output)
print('Native lower LCD: theme, log/scores toggle and walk feedback verified')

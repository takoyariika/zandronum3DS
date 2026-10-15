"""Convert the port's actual rotated BGR LCD buffer dump to a PNG (stdlib only)."""
import argparse,struct,zlib
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__); p.add_argument('input',type=Path); p.add_argument('output',type=Path)
a=p.parse_args(); data=a.input.read_bytes(); stride,width=struct.unpack_from('<HH',data)
assert stride==240 and width in (320,400,800),'Unsupported LCD geometry'
pixel_bytes=2 if a.input.suffix=='.rgb565' else 3
assert len(data)==4+stride*width*pixel_bytes,'Incomplete LCD dump'
rgb=bytearray()
for y in range(stride):
    rgb.append(0)
    for x in range(width):
        offset=4+(x*stride+(stride-1-y))*pixel_bytes
        if pixel_bytes==2:
            color=struct.unpack_from('<H',data,offset)[0]
            rgb+=bytes((((color>>11)&31)*255//31,((color>>5)&63)*255//63,(color&31)*255//31))
        else:
            rgb+=data[offset:offset+3][::-1]
def chunk(kind,body):
    return struct.pack('>I',len(body))+kind+body+struct.pack('>I',zlib.crc32(kind+body)&0xffffffff)
a.output.write_bytes(b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',width,stride,8,2,0,0,0))+chunk(b'IDAT',zlib.compress(rgb))+chunk(b'IEND',b''))
print('Actual LCD framebuffer:',width,'x',stride,'nonblack pixels=',sum(any(data[i:i+pixel_bytes]) for i in range(4,len(data),pixel_bytes)))

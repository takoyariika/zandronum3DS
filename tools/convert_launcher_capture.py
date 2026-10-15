"""Convert native diagnostic RGB565 captures into a dual-screen preview."""
from pathlib import Path
import struct
import sys
from PIL import Image

def screen(path):
    data=Path(path).read_bytes()
    width,height=struct.unpack('<HH',data[:4])
    assert width in (320,400) and height==240 and len(data)==4+width*height*2
    image=Image.new('RGB',(width,height))
    pixels=[]
    for y in range(height):
        for x in range(width):
            color=struct.unpack_from('<H',data,4+2*(x*height+height-1-y))[0]
            pixels.append(((color>>11)*255//31,((color>>5)&63)*255//63,(color&31)*255//31))
    image.putdata(pixels)
    return image

top,bottom=screen(sys.argv[1]),screen(sys.argv[2])
canvas=Image.new('RGB',(400,492),(13,19,29))
canvas.paste(top,(0,0)); canvas.paste(bottom,(40,252))
canvas.resize((800,984),Image.Resampling.NEAREST).save(sys.argv[3])

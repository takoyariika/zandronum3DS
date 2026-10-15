"""Stamp actual pixel access, including textures retained directly by the renderer."""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
count=0
for path in (root/'upstream/src/textures').glob('*.cpp'):
    text=path.read_bytes().decode('latin-1').replace('\r\n','\n')
    pattern=r'(const BYTE \*\w+::Get(?:Pixels|Column)\s*\([^\n]*\)\s*\n\{)(?!\n\s*Z3DS_TOUCH_TEXTURE)'
    text,n=re.subn(pattern,r'\1\n\tZ3DS_TOUCH_TEXTURE(this);',text)
    if n: path.write_bytes(text.encode('latin-1')); count+=n
print('Actual texture access stamps added:',count)

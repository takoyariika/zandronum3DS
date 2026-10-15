"""Download and embed the official ISRG Root X2 alongside the existing X1."""
from pathlib import Path
import hashlib
import urllib.request
ROOT=Path(__file__).resolve().parents[1]
URL='https://letsencrypt.org/certs/isrg-root-x2.der'
der=urllib.request.urlopen(URL,timeout=30).read()
assert len(der)<4096 and der[:1]==b'\x30'
digest=hashlib.sha256(der).hexdigest()
assert digest=='69729b8e15a86efc177a57afb7171dfc64add28c2fca8cf1507e34453ccb1470', 'Unexpected ISRG Root X2 certificate'
out=ROOT/'platform/3ds/doomlist_ca_3ds.h'
text=out.read_text()
marker='// ISRG Root X2'
if marker in text: text=text[:text.index(marker)]
text+=f'\n// ISRG Root X2 from {URL}\n// SHA256: {digest}\nstatic const unsigned char doomlistRootX2[]={{\n'
for start in range(0,len(der),16): text+='    '+','.join(f'0x{b:02x}' for b in der[start:start+16])+',\n'
text+='};\n'
out.write_text(text)
print(digest)

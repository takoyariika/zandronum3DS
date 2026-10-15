"""Record canonical release resource bytes, including intentionally mixed newlines."""
import argparse, hashlib, json, subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser()
p.add_argument('--git',required=True)
a=p.parse_args()
files=[]
for folder in ('wadsrc','wadsrc_st'):
    files.extend(sorted(f.relative_to(root/'upstream').as_posix()
                        for f in (root/'upstream'/folder/'static').rglob('*') if f.is_file()))
requests=''.join('HEAD:'+f+'\n' for f in files).encode()
result=subprocess.run([a.git,'-C',str(root/'upstream'),'cat-file','--batch'],input=requests,stdout=subprocess.PIPE,check=True).stdout
pos=0; manifest={}
for f in files:
    end=result.index(b'\n',pos); fields=result[pos:end].split()
    assert fields[1]==b'blob',f
    size=int(fields[2]); data=result[end+1:end+1+size]; pos=end+size+2
    manifest[f]=hashlib.sha256(data).hexdigest()
(root/'reference-generated/engine-resource-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('Canonical engine resources:',len(manifest))

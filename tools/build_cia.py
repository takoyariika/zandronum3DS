"""Create a New-3DS 124 MiB CIA using the tested ELF (no game data in RomFS)."""
import ast
from pathlib import Path
import subprocess
ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT/'build-3ds'
DIST = OUT/'dist'
# Retain the reference project's standard libctru RSF service/dependency mapping.
source = ROOT/'reference-zdoom3ds/tools/gen_cia.py'
saved = ROOT/'reference-generated/cia_config.py'
if source.exists():
    tree = ast.parse(source.read_text())
    wanted = {'SERVICES', 'SYSCALLS', 'SYSCALL_IDS', 'DEPENDENCIES'}
    nodes = []
    for node in tree.body:
        if isinstance(node, ast.Assign) and any(isinstance(t, ast.Name) and t.id in wanted for t in node.targets): nodes.append(node)
        if isinstance(node, ast.FunctionDef) and node.name == 'rsf': nodes.append(node)
    saved.write_text('# Adapted RSF mappings from Ultimate Doom Launcher 3DS; see sources.lock.json.\n'+ast.unparse(ast.Module(body=nodes, type_ignores=[]))+'\n')
namespace = {}
exec(compile(saved.read_text(), str(saved), 'exec'), namespace)
# TLS 1.2 browser seeds its RNG with the 3DS PS service; camera is unused.
namespace['SERVICES']=[('ps:ps' if service=='cam:u' else service) for service in namespace['SERVICES']]
romfs = OUT/'romfs'; romfs.mkdir(exist_ok=True)
values = {'app_ram_mb':124, 'cpu':'804MHz + L2', 'short_title':'Zan3DS',
          'product_code':'CTR-H-ZN3D', 'unique_id':'0xFD733', 'fs_access':'DirectSdmc DirectSdmcWrite CategorySystemApplication',
          'system_mode':'64MB', 'stack_kb':1024}
rsf = OUT/'zandronum.rsf'
rsf.write_text(namespace['rsf'](values).replace('build/romfs', 'build-3ds/romfs'))
makerom = ROOT/'dependencies/makerom-0.18.3/makerom.exe'
# CIA stores micro in four bits; carry into minor for monotonic updates.
port_patch=18
r = subprocess.run([str(makerom), '-f', 'cia', '-o', str(DIST/'zandronum.cia'),
                    '-elf', str(DIST/'zandronum.elf'), '-rsf', str(rsf),
                    '-icon', str(DIST/'zandronum.smdh'), '-exefslogo', '-target', 't',
                    '-major','0','-minor',str(1+port_patch//16),'-micro',str(port_patch%16)], cwd=ROOT,
                   stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
(OUT/'makerom.log').write_text(r.stdout)
print(r.stdout)
if r.returncode: raise SystemExit(r.returncode)
print(DIST/'zandronum.cia')

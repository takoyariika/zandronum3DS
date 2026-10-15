"""Package the generic launcher without original games or MODs."""
import json
from pathlib import Path
import subprocess
import zipfile
import shutil
import os
ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT/'build-3ds'
DIST = OUT/'dist'
git = os.environ.get('GIT', 'git')
if (ROOT/'upstream/.git').exists():
    patch = subprocess.check_output([git, '-c', 'core.safecrlf=false', '-C', str(ROOT/'upstream'), 'diff', '--binary'], stderr=subprocess.DEVNULL)
else:
    patch = (ROOT/'zandronum-3ds.patch').read_bytes()
(OUT/'zandronum-3ds.patch').write_bytes(patch)
instructions = '''Zandronum 3DS alpha 0.1.18 (Zandronum 3.2.1)

Target: New 3DS / New 3DS XL / New 2DS XL with custom firmware.
Extract this ZIP to the SD root, then install cia/zandronum.cia with FBI.
Supply your own supported base game IWAD in SD:/3ds/zandronum/.
MODs are optional. No particular MOD is required or selected by default.
When updating, replace args.txt too. Existing launch.txt selections are kept;
press X in the launcher to clear selected MODs and play the base game alone.
Game menu Quit returns to the file launcher after engine/network cleanup.
Choose another game or network server there. B or the bottom-right
"B Exit to HOME" touch button exits the application to the 3DS HOME menu.

D-pad/L/R browse, A selects an IWAD or toggles a MOD, START plays offline.
Y opens Network mode. Y in the server list searches using the system keyboard.
SELECT enters host:port (default10666). A shows required files; X downloads
missing MODs; START connects after all required files are available.
Known list addresses use the server's ordered files/passwords. Unlisted
addresses use the current launcher selection. Original game IWADs are supplied
by their owner; they are never downloaded by this launcher.

Downloads stay in SD:/3ds/zandronum/downloads/ after exit and reboot. The
launcher rescans this directory and reuses saved files across servers and
offline play. Existing files are kept. Do not delete downloads/ when updating.
Large files use two simultaneous HTTPS streams when the mirror supports byte
ranges and a stable ETag/Last-Modified validator; otherwise one stream is used.
SD writes are buffered, UI shows KiB/s and connection count, B/touch cancels.
There is no guaranteed speed multiplier: Wi-Fi, server and TLS/SD speed matter.
To force one connection, rename args-download-single.txt to args.txt.
Maximum512 MiB per file; completed length and PWAD/PK3 magic are checked.
Failed/cancelled partial files are removed; completed files remain.
Certificate/hostname verification is mandatory; keep the console date correct.

Keep all three engine files together in SD:/3ds/zandronum/:
zandronum.pk3
zandronum.pk3.3ds-resources
zandronum.pk3.3ds-cache
These cache files load automatically; never select them as MODs.
The launcher scans WAD/PK3/ZIP files up to three subdirectory levels.
Selections are saved in launch.txt. Config/saves/downloads remain on SD.
Normal startup uses no diagnostic/test flags. Offline play creates no SOC
sockets/DNS requests. Add -3ds-diagnostics only when collecting diagnostics.

Circle Pad: forward/back and turn left/right. C-stick/touchscreen: look.
A/L: main attack; B: jump; Y: crouch; X: use; R: alternate attack.
ZL/D-pad down: previous weapon; ZR/D-pad up: next weapon.
D-pad left/right: strafe. START: menu; SELECT: automap.
Always run is enabled once on first launch of0.1.17 or later; saved changes are kept.
Bottom right: hold to walk. Release/slide out/menu/resume cancels walking.
Bottom left: toggle recent log / player scores. Tap score footer for next page.
The bottom screen uses the launcher navy/teal theme. Drag below buttons to look.
Menus keep A confirm/B back and D-pad navigation. More: Customize Controls.
The first launch migrates old button/axis mappings once; later customization
is saved. No config deletion is needed.
Nested input/APT processing during redraw or audio restore is guarded.
Unexpected HOME return around WIN/results on physical hardware is still under
investigation. On recurrence, save transition.txt, apt-exit.txt (if present),
quit.txt (if present), fatal.txt (if present), and the latest engine log BEFORE
relaunching: per-session diagnostics are cleared on the next startup.
Scene/exit diagnostics are event-based; no periodic gameplay writes were added.
While spectating online, A opens the join menu; A confirms joining. The HUD
shows A instead of Space. Use the D-pad/A for class/team selection if required.
DSP firmware is extracted from this console if needed; no firmware is included.
Account authentication, UPnP and movie/CD playback are unavailable.
Fatal errors are saved to fatal.txt; START on the error screen returns HOME.

Optional resource caches: drag your WAD/PK3/ZIP onto tools/Build-ResourceCache.cmd
on Windows; place the generated caches beside the unchanged original file.
For a smaller PNG-only cache, use tools/Build-TextureCache.cmd.
Both tools accept WADs and generate .3ds-resources for all WAD lumps.
For example: DOOM2.WAD -> DOOM2.WAD.3ds-resources (keep both on SD).
Caches are detected automatically; do not add them to args.txt.
Regenerate after changing an original. ZIP64/encrypted ZIPs are unsupported.

The engine uses software rendering at400x240. Real hardware download speed,
audio and keyboard interaction need verification; Citra tests cannot establish
real-device FPS or Wi-Fi throughput. This experimental package contains no
original game IWADs or third-party MODs. Build instructions: README.md in source.
'''
(DIST/'INSTALL.txt').write_text(instructions)
tree = OUT/'sdmc/3ds/zandronum'
with zipfile.ZipFile(DIST/'zandronum-3ds-alpha.zip', 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as z:
    z.write(DIST/'zandronum.3dsx', '3ds/zandronum.3dsx')
    if (DIST/'zandronum.cia').exists(): z.write(DIST/'zandronum.cia', 'cia/zandronum.cia')
    for name in ('zandronum.pk3', 'skulltag_actors.pk3'):
        z.write(tree/name, '3ds/zandronum/'+name)
    engine_cache = tree/'zandronum.pk3.3ds-cache'
    z.write(engine_cache, '3ds/zandronum/'+engine_cache.name)
    engine_resources = tree/'zandronum.pk3.3ds-resources'
    z.write(engine_resources, '3ds/zandronum/'+engine_resources.name)
    z.writestr('3ds/zandronum/args.txt', '+native_resource_trim_3ds 1 +native_titlemap_3ds 1\n')
    z.writestr('3ds/zandronum/args-static-title.txt', '+native_resource_trim_3ds 1 +native_titlemap_3ds 0\n')
    z.writestr('3ds/zandronum/args-animated-title.txt', '+native_resource_trim_3ds 1 +native_titlemap_3ds 1\n')
    z.writestr('3ds/zandronum/args-doom.txt', '\n')
    z.writestr('3ds/zandronum/args-download-single.txt', '-3ds-download-single +native_resource_trim_3ds 1 +native_titlemap_3ds 1\n')
    z.writestr('3ds/zandronum/args-display-safe.txt', '+native_resource_trim_3ds 0 +native_titlemap_3ds 0\n')
    z.writestr('INSTALL.txt', instructions)
    for name in ('Build-TextureCache.ps1', 'Build-TextureCache.cmd', 'Build-ResourceCache.ps1', 'Build-ResourceCache.cmd', 'CACHE-README.txt', 'convert_video_dump.py'):
        z.write(ROOT/'tools'/name, 'tools/'+name)
    z.write(ROOT/'dependencies/AUDIO_LICENSES.txt', 'licenses/AUDIO_LICENSES.txt')
    z.write(ROOT/'dependencies/mbedtls-3.6.7/LICENSE', 'licenses/MBEDTLS_LICENSE.txt')
    for file in (ROOT/'upstream').glob('LICENSE*'): z.write(file, 'licenses/'+file.name)
    for file in (ROOT/'upstream/docs').rglob('*'):
        if file.is_file() and ('lic' in file.name.lower() or 'copying' in file.name.lower()):
            z.write(file, 'licenses/'+file.relative_to(ROOT/'upstream/docs').as_posix())
with zipfile.ZipFile(DIST/'zandronum-3ds-source.zip', 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as z:
    for folder in ('upstream', 'platform', 'tools', 'dependencies', 'reference-generated'):
        for file in sorted((ROOT/folder).rglob('*')):
            if not file.is_file() or '.git' in file.parts or '__pycache__' in file.parts or file.suffix.lower() == '.exe': continue
            z.write(file, file.relative_to(ROOT).as_posix())
    for name in ('README.md', 'BUILDING.md', 'NOTICE.md', 'LICENSE', 'RELEASE_NOTES.md', '.gitignore', '.gitattributes', 'sources.lock.json', 'PORT_STATUS.md'):
        if not (ROOT/name).exists(): continue
        z.write(ROOT/name, name)
    z.write(OUT/'zandronum-3ds.patch', 'zandronum-3ds.patch')
print(DIST/'zandronum-3ds-alpha.zip')
print(DIST/'zandronum-3ds-source.zip')
with zipfile.ZipFile(DIST/'zandronum-engine-caches.zip', 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as z:
    for name in ('zandronum.pk3','zandronum.pk3.3ds-cache','zandronum.pk3.3ds-resources'):
        z.write(tree/name,'3ds/zandronum/'+name)
shutil.copyfile(DIST/'zandronum-3ds-alpha.zip', DIST/'zandronum-3ds-alpha-0.1.18.zip')
shutil.copyfile(DIST/'zandronum.cia', DIST/'zandronum-0.1.18.cia')

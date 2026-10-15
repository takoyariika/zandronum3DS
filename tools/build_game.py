"""Build the bundled libraries and link the experimental Zandronum 3DS executable.
Run arm_compile.py first to create build-config.json and engine objects.
"""
import argparse
import concurrent.futures
import json
import pathlib
import subprocess
import re
import shlex

ROOT = pathlib.Path(__file__).resolve().parents[1]
OUT = ROOT/'build-3ds'

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--jobs', type=int, default=6)
    p.add_argument('--libs-only', action='store_true')
    p.add_argument('--trace-dumb-heap', action='store_true', help='Diagnostic allocation callsites for the tracker music library')
    a = p.parse_args()
    cfg = json.loads((OUT/'build-config.json').read_text())
    sdk = pathlib.Path(cfg['devkitpro'])
    # Refresh changed engine/platform units before linking; retain the full audit.
    if not a.libs_only:
        for unit in cfg['units']:
            source = pathlib.Path(unit)
            obj = OUT/'obj'/source.relative_to(ROOT).with_suffix('.o')
            if not obj.exists() or source.stat().st_mtime_ns > obj.stat().st_mtime_ns:
                r = subprocess.run([cfg['compiler'], *cfg['flags'], '-c', str(source), '-o', str(obj)], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, errors='replace')
                log = OUT/'logs'/source.relative_to(ROOT).with_suffix('.txt')
                log.write_text(r.stdout)
                if r.returncode: print(r.stdout); return 1
    ref = json.loads((ROOT/'reference-generated/build.json').read_text())
    modules = {m['module']: m for m in ref['Modules']}
    arch = ['-march=armv6k', '-mtune=mpcore', '-mfloat-abi=hard', '-mtp=soft']
    jobs = []
    sources = [s for s in ref['Sources'] if s['module'] not in ('lib:dumb', 'lib:lzma', 'lib:gme')]
    for mod, folder, pattern in [('lib:dumb', 'dumb', r'add_library\( dumb(.*?)\)'),
                                  ('lib:lzma', 'lzma', r'set\( LZMA_FILES(.*?)\)')]:
        cm = (ROOT/'upstream'/folder/'CMakeLists.txt').read_text()
        block = re.search(pattern, cm, re.S).group(1)
        for unit in re.findall(r'[\w/]+\.(?:c|cpp)\b', block):
            sources.append({'unit': 'engine/zdoom/'+folder+'/'+unit, 'module': mod, 'compile': 'yes'})
    cm = (ROOT/'upstream/game-music-emu/gme/CMakeLists.txt').read_text()
    for unit in sorted(set(re.findall(r'[\w]+\.cpp\b', cm))):
        if (ROOT/'upstream/game-music-emu/gme'/unit).exists():
            sources.append({'unit': 'engine/zdoom/game-music-emu/gme/'+unit, 'module': 'lib:gme', 'compile': 'yes'})
    for s in sources:
        if not s['module'].startswith('lib:') or s.get('compile') != 'yes': continue
        unit = s['unit'].replace('engine/zdoom/', 'upstream/')
        source = ROOT/unit
        if not source.exists(): raise FileNotFoundError(source)
        m = modules[s['module']]
        cpp = source.suffix in ('.cpp', '.cc')
        flags = [*arch, '-O2', '-g', '-fno-strict-aliasing', '-D__3DS__']
        if cpp: flags += ['-std=gnu++14', '-fno-rtti']
        else: flags += ['-std=gnu89' if s['module'] == 'lib:gdtoa' else '-std=gnu99']
        flags += ['-I'+str(ROOT/'reference-generated/config')]
        for inc in m.get('includes', '').split():
            if inc != '~': flags.append('-I'+str(ROOT/inc.replace('engine/zdoom/', 'upstream/').replace('platform/3ds/config', 'reference-generated/config')))
        flags += [v for v in m.get('cflags', '').split() if v != '~']
        if a.trace_dumb_heap and s['module']=='lib:dumb' and not cpp:
            flags += ['-Dmalloc=Z3DS_DumbMalloc', '-Dcalloc=Z3DS_DumbCalloc', '-Drealloc=Z3DS_DumbRealloc', '-Dfree=Z3DS_DumbFree']
        jobs.append((source, flags))
    tls=ROOT/'dependencies/mbedtls-3.6.7'
    tls_flags=[*arch,'-std=gnu99','-O2','-DMBEDTLS_CONFIG_FILE="mbedtls_config_3ds.h"','-I'+str(ROOT/'platform/3ds'),'-I'+str(tls/'include'),'-I'+str(tls/'library')]
    for source in sorted((tls/'library').glob('*.c')):
        jobs.append((source,tls_flags))
    jobs += [(ROOT/'dependencies/sqlite-amalgamation-3360000/sqlite3.c', [*arch, '-std=gnu99', '-O2', '-DSQLITE_THREADSAFE=0', '-DSQLITE_OMIT_LOAD_EXTENSION', '-DSQLITE_OS_OTHER=0', '-DSQLITE_MAX_MMAP_SIZE=0', '-DHAVE_MREMAP=0'])]
    # Zandronum-specific libraries absent from the ZDoom reference build.
    jobs += [(ROOT/'upstream/GeoIP/GeoIP.c', [*arch, '-std=gnu99', '-O2', '-DPACKAGE_VERSION="1.4.5"', '-DHAVE_GETTIMEOFDAY', '-I'+str(ROOT/'upstream/GeoIP')])]
    jobs.append((ROOT/'platform/3ds/posix_stubs_3ds.c', [*arch, '-std=gnu99', '-O2']))
    codec_flags=[*arch, '-std=gnu99', '-O2', '-I'+str(ROOT/'dependencies/libogg/include'), '-I'+str(ROOT/'dependencies/tremor')]
    for name in ('bitwise','framing'):
        jobs.append((ROOT/('dependencies/libogg/src/'+name+'.c'),codec_flags))
    for name in ('mdct','block','window','synthesis','info','floor1','floor0','vorbisfile','res012','mapping0','registry','codebook','sharedbook'):
        jobs.append((ROOT/('dependencies/tremor/'+name+'.c'),codec_flags))
    def compile_one(job):
        source, flags = job
        rel = source.relative_to(ROOT)
        obj = OUT/'obj'/rel.with_suffix('.o')
        obj.parent.mkdir(parents=True, exist_ok=True)
        log = OUT/'logs'/rel.with_suffix('.txt')
        log.parent.mkdir(parents=True, exist_ok=True)
        compiler = sdk/'devkitARM/bin'/('arm-none-eabi-g++.exe' if source.suffix in ('.cpp', '.cc') else 'arm-none-eabi-gcc.exe')
        flags = [f for f in flags if f != '-D_7ZIP_PPMD_SUPPPORT']
        flags += ['-D__3DS__', '-I'+str(sdk/'libctru/include'), '-I'+str(ROOT/'platform/3ds')]
        cache = obj.with_suffix('.command.json')
        dependencies = obj.with_suffix('.d')
        stamp = {'source_mtime': source.stat().st_mtime_ns, 'flags': flags}
        headers_current = False
        if obj.exists() and dependencies.exists():
            contents = dependencies.read_text().replace('\\\n', '')
            dep_names = shlex.split(contents.split(': ', 1)[1])
            headers_current = all(pathlib.Path(n).exists() and pathlib.Path(n).stat().st_mtime_ns <= obj.stat().st_mtime_ns for n in dep_names)
        if obj.exists() and cache.exists() and headers_current and json.loads(cache.read_text()) == stamp:
            return {'source': str(rel), 'ok': True, 'object': str(obj), 'errors': []}
        r = subprocess.run([str(compiler), *flags, '-MMD', '-MF', str(dependencies), '-c', str(source), '-o', str(obj)], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, errors='replace')
        log.write_text(r.stdout)
        if r.returncode == 0: cache.write_text(json.dumps(stamp))
        return {'source': str(rel), 'ok': r.returncode == 0, 'object': str(obj), 'errors': [l for l in r.stdout.splitlines() if 'error:' in l][:4]}
    with concurrent.futures.ThreadPoolExecutor(max_workers=a.jobs) as pool:
        results = list(pool.map(compile_one, jobs))
    (OUT/'library-report.json').write_text(json.dumps(results, indent=2))
    print(f"Libraries compiled {sum(r['ok'] for r in results)}/{len(results)}")
    for r in results:
        if not r['ok']: print(r['source'], r['errors'])
    if any(not r['ok'] for r in results): return 1
    if a.libs_only: return 0
    report = json.loads((OUT/'compile-report.json').read_text())
    if report['compiled'] != report['units']:
        print('Engine compilation incomplete; link refused.'); return 1
    units = [pathlib.Path(u) for u in cfg['units']]
    units.sort(key=lambda u: 0 if u.name == '__autostart.cpp' else 2 if u.name == 'zzautozend.cpp' else 1)
    objects = [str(OUT/'obj'/u.relative_to(ROOT).with_suffix('.o')) for u in units]
    objects += [r['object'] for r in results]
    dist = OUT/'dist'; dist.mkdir(exist_ok=True)
    elf = dist/'zandronum.elf'
    response = OUT/'link.rsp'
    response.write_text('\n'.join('"'+s.replace('\\', '/')+'"' for s in objects))
    r = subprocess.run([cfg['compiler'], *arch, '-specs=3dsx.specs', '-Wl,--no-gc-sections', '-Wl,-Map,'+str(dist/'zandronum.map'), '@'+str(response), '-L'+str(sdk/'libctru/lib'), '-lctru', '-lm', '-o', str(elf)], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, errors='replace')
    (OUT/'link.log').write_text(r.stdout)
    if r.returncode:
        print(r.stdout[-12000:]); return 1
    smdh = dist/'zandronum.smdh'
    subprocess.run([str(sdk/'tools/bin/smdhtool.exe'), '--create', 'Zandronum 3DS', 'Zandronum 3.2.1 - native audio and multiplayer', 'Zandronum / 3DS port', str(sdk/'libctru/default_icon.png'), str(smdh)], check=True)
    subprocess.run([str(sdk/'tools/bin/3dsxtool.exe'), str(elf), str(dist/'zandronum.3dsx'), '--smdh='+str(smdh)], check=True)
    report['linked'] = True
    report['configuration'] = 'Experimental 3DS game build: software rendering, NDSP/CSND audio, account login and UPnP unavailable'
    (OUT/'compile-report.json').write_text(json.dumps(report, indent=2))
    print(dist/'zandronum.3dsx')
    return 0

if __name__ == '__main__':
    raise SystemExit(main())

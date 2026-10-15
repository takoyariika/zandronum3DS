"""Compile Zandronum translation units with devkitARM; report real port blockers.

This is a compile audit, not a linkable game build. No desktop compiler fallback.
Run: python tools/arm_compile.py --devkitpro PATH [--jobs 4] [--filter TEXT]
"""
import argparse
import concurrent.futures
import json
import pathlib
import re
import subprocess
import sys
import time
from datetime import datetime, timezone

ROOT = pathlib.Path(__file__).resolve().parents[1]

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--devkitpro', type=pathlib.Path, required=True)
    p.add_argument('--jobs', type=int, default=4)
    p.add_argument('--filter', default='')
    p.add_argument('--git', default='git', help='Native Windows git executable for source metadata')
    a = p.parse_args()
    src = ROOT / 'upstream/src'
    cm = (src / 'CMakeLists.txt').read_text()
    block = re.search(r'add_executable\( zdoom WIN32(.*?)\n\)', cm, re.S).group(1)
    units = []
    for line in block.splitlines():
        token = line.split('#')[0].strip()
        if token.endswith(('.cpp', '.c')) and '${' not in token:
            if token not in ('network/srp.cpp', 'm_specialpaths.cpp'):
                units.append(src / token)
    units.append(src / 'sdl/glstubs.cpp')
    units.append(src / 'sdl/i_cd.cpp')
    units.append(src / 'sdl/i_movie.cpp')
    units += [u for u in (ROOT/'platform/3ds').glob('*.cpp') if u.name != 'probe.cpp']
    units = [u for u in units if a.filter in str(u)]
    out = ROOT / 'build-3ds'
    out.mkdir(exist_ok=True)
    grammar = src/'xlat/xlat_parser.y'
    reference_grammar = ROOT/'reference-generated/xlat_parser.y'
    if grammar.read_text().replace('\r\n', '\n') != reference_grammar.read_text().replace('\r\n', '\n'):
        raise RuntimeError('XLAT grammar differs: regenerate parser from Zandronum before compiling')
    if not (out/'sc_man_scanner.h').exists():
        raise RuntimeError('Generate build-3ds/sc_man_scanner.h with upstream re2c first; see README.md')
    if (ROOT/'upstream/.git').exists():
        commit = subprocess.check_output([a.git, '-C', str(ROOT/'upstream'), 'rev-parse', 'HEAD'], text=True).strip()
        timestamp = subprocess.check_output([a.git, '-C', str(ROOT/'upstream'), 'show', '-s', '--format=%ct', 'HEAD'], text=True).strip()
    else:
        locked = json.loads((ROOT/'sources.lock.json').read_text())['zandronum']
        commit, timestamp = locked['commit'], str(locked['timestamp'])
    revision_time = datetime.fromtimestamp(int(timestamp), timezone.utc).strftime('%y%m%d-%H%M')
    (out/'gitinfo.h').write_text('#define HG_REVISION_HASH_STRING "' + commit + '"\n#define HG_REVISION_NUMBER ' + timestamp + '\n#define HG_TIME "' + revision_time + '"\n')
    subprocess.run([sys.executable, str(ROOT/'upstream/protocolspec/generator/codegenerator.py'),
                    '--spec', str(ROOT/'upstream/protocolspec/spec.txt'), '--source', str(src/'network/servercommands.cpp'),
                    '--header', str(src/'network/servercommands.h')], check=True, stdout=subprocess.DEVNULL)
    compiler = a.devkitpro / 'devkitARM/bin/arm-none-eabi-g++.exe'
    version = subprocess.check_output([str(compiler), '--version'], text=True).splitlines()[0]
    flags = ['-march=armv6k', '-mtune=mpcore', '-mfloat-abi=hard', '-mtp=soft',
             '-std=gnu++14', '-O2', '-fno-strict-aliasing', '-fno-rtti',
             '-D__3DS__', '-D__unix__', '-DNOASM', '-DDISABLE_SSE', '-DNO_GL=1', '-DNO_SOUND=1',
             '-DMBEDTLS_CONFIG_FILE="mbedtls_config_3ds.h"',
             '-DNO_GTK=1', '-DNO_SERVER_GUI=1', '-DNO_LIBSECRET=1',
             '-D__forceinline=inline', '-Dstricmp=strcasecmp', '-Dstrnicmp=strncasecmp',
             '-Wno-narrowing', '-Wno-register', '-Wno-deprecated-declarations']
    includes = [ROOT / 'platform/3ds', ROOT/'dependencies/mbedtls-3.6.7/include', out, src, src / 'sdl',
                ROOT / 'dependencies/tremor', ROOT / 'dependencies/libogg/include', ROOT / 'dependencies/dr_libs',
                ROOT / 'reference-generated',
                ROOT / 'reference-generated/config',
                ROOT / 'upstream/zlib',
                ROOT / 'dependencies/sqlite-amalgamation-3360000',
                ROOT / 'dependencies/zstd-1.5.7/lib',
                a.devkitpro / 'libctru/include', a.devkitpro / 'portlibs/3ds/include/SDL']
    includes += [src / n for n in ['g_doom', 'g_heretic', 'g_hexen', 'g_raven',
                 'g_strife', 'g_shared', 'oplsynth', 'sound', 'textures', 'thingdef', 'timidity', 'xlat']]
    includes += [src / 'huffman', src / 'Linux']
    includes += [ROOT / 'upstream' / n for n in ['gdtoa', 'dumb/include', 'game-music-emu', 'bzip2', 'jpeg-6b', 'lzma/C', 'sqlite', 'rnnoise', 'upnpnat']]
    flags += ['-I' + str(i) for i in includes]
    (out/'build-config.json').write_text(json.dumps({'compiler': str(compiler), 'flags': flags,
        'units': [str(u) for u in units], 'devkitpro': str(a.devkitpro)}, indent=2))
    def compile_one(u):
        rel = u.relative_to(ROOT)
        obj = out / 'obj' / rel.with_suffix('.o')
        obj.parent.mkdir(parents=True, exist_ok=True)
        log = out / 'logs' / rel.with_suffix('.txt')
        log.parent.mkdir(parents=True, exist_ok=True)
        start = time.time()
        r = subprocess.run([str(compiler), *flags, '-c', str(u), '-o', str(obj)],
                           stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, errors='replace')
        log.write_text(r.stdout, encoding='utf-8')
        errors = [l for l in r.stdout.splitlines() if 'error:' in l]
        return {'source': str(rel).replace('\\', '/'), 'ok': r.returncode == 0,
                'seconds': round(time.time() - start, 2), 'errors': errors[:8]}
    with concurrent.futures.ThreadPoolExecutor(max_workers=a.jobs) as pool:
        results = list(pool.map(compile_one, units))
    report = {'compiler': version, 'upstream_commit': commit, 'linked': False,
              'configuration': 'Software rendering, NO_SOUND compile audit; no audio backend or platform startup linked', 'units': len(results),
              'compiled': sum(r['ok'] for r in results), 'results': results}
    (out / 'compile-report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
    print(f"ARM compiled {report['compiled']}/{report['units']} units; no link attempted.")
    for r in results:
        if not r['ok']:
            print(r['source'] + ': ' + (r['errors'][0] if r['errors'] else 'see log'))
    return 0 if all(r['ok'] for r in results) else 1

if __name__ == '__main__':
    raise SystemExit(main())

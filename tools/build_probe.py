"""Build a 3DS diagnostic with native Windows devkitARM tools (no MSYS)."""
import argparse
import pathlib
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[1]

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--devkitpro', type=pathlib.Path, required=True)
    a = p.parse_args()
    d = a.devkitpro
    out = ROOT / 'build-3ds/dist'
    out.mkdir(parents=True, exist_ok=True)
    arch = ['-march=armv6k', '-mtune=mpcore', '-mfloat-abi=hard', '-mtp=soft']
    cxx = d / 'devkitARM/bin/arm-none-eabi-g++.exe'
    obj, elf, rom = [out / ('zandronum-probe.' + x) for x in ('o', 'elf', '3dsx')]
    subprocess.run([str(cxx), *arch, '-std=gnu++14', '-O2', '-D__3DS__',
                    '-I'+str(d/'libctru/include'), '-c', str(ROOT/'platform/3ds/probe.cpp'), '-o', str(obj)], check=True)
    subprocess.run([str(cxx), *arch, '-specs=3dsx.specs', str(obj),
                    '-L'+str(d/'libctru/lib'), '-lctru', '-lm', '-o', str(elf)], check=True)
    subprocess.run([str(d/'tools/bin/3dsxtool.exe'), str(elf), str(rom)], check=True)
    print(rom)

if __name__ == '__main__':
    main()

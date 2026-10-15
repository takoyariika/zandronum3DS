import json,pathlib,subprocess
root=pathlib.Path(__file__).resolve().parents[1]
cfg=json.loads((root/'build-3ds/build-config.json').read_text())
for name in ('audio_decoder_3ds','ndspsound','csnd_3ds','dspdump_3ds','vorbis_decoder'):
    source=root/('platform/3ds/'+name+'.cpp')
    obj=root/('build-3ds/obj/platform/3ds/'+name+'.o')
    result=subprocess.run([cfg['compiler'],*cfg['flags'],'-c',str(source),'-o',str(obj)],capture_output=True,text=True,errors='replace')
    (root/('build-3ds/logs/platform/3ds/'+name+'.txt')).write_text(result.stdout+result.stderr)
    print(name,result.returncode,result.stderr[-10000:] if result.returncode else '')

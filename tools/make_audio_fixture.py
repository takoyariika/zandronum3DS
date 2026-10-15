"""Create original sine tones for testing all native decoders, without game assets."""
from pathlib import Path
import struct,subprocess
root=Path(__file__).resolve().parents[1]
out=root/'build-3ds/audio-fixture'; out.mkdir(exist_ok=True)
lumps=[('SNDINFO',b'diag/wav DSNWAV\ndiag/ogg DSNOGG\ndiag/mp3 DSNMP3\ndiag/flac DSNFLAC\n')]
for fmt,codec in [('wav','pcm_s16le'),('ogg','libvorbis'),('mp3','libmp3lame'),('flac','flac')]:
    file=out/('tone.'+fmt)
    subprocess.run([r'C:\ffmpeg\bin\ffmpeg.exe','-v','error','-y','-f','lavfi','-i','sine=frequency=440:sample_rate=22050:duration=0.5','-ac','1','-c:a',codec,str(file)],check=True)
    lumps.append(('DSN'+fmt.upper(),file.read_bytes()))
lumps.append(('DIAGMUS',(out/'tone.ogg').read_bytes()))
body=bytearray(); directory=bytearray()
for name,data in lumps:
    directory+=struct.pack('<ii8s',12+len(body),len(data),name.encode().ljust(8,b'\0')); body+=data
(root/'build-3ds/sdmc/3ds/zandronum/audio-fixture.wad').write_bytes(struct.pack('<4sii',b'PWAD',len(lumps),12+len(body))+body+directory)
print('Native decoder fixture generated.')

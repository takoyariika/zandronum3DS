"""Adapt the pinned 3DS reference backend to Zandronum's older audio ABI.
Only run for initial import; the resulting platform files are maintained source.
"""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
ref=root/'reference-zdoom3ds/platform/3ds'
out=root/'platform/3ds'
for name in ('csnd_3ds.cpp','csnd_3ds.h','dspdump_3ds.cpp'):
    (out/name).write_text((ref/name).read_text())
decoder=(ref/'vorbis_decoder.cpp').read_text().replace('#include <tremor/ivorbisfile.h>','#include "ivorbisfile.h"').replace('#include "i_soundinternal.h"','#include "audio_decoder_3ds.h"')
(out/'vorbis_decoder.cpp').write_text(decoder)
s=(ref/'ndspsound.cpp').read_text()
s=s.replace('#include "i_soundinternal.h"','#include "i_soundinternal.h"\n#include "audio_decoder_3ds.h"\n#include "md5.h"\n#include <sys/stat.h>')
s=s.replace('#include "bd3ds_sheets.h"','#include "bd3ds_sheets.h"\n#define BD3DS_PATH_APP_DIR "sdmc:/3ds/zandronum/"')
s=s.replace('\t\tchan->ManualRolloff = manual;','\t\t(void)manual;')
s=s.replace('SoundDecoder *CreateDecoder(FileReader *reader) { return SoundRenderer::CreateDecoder(reader); }','SoundDecoder *CreateDecoder(FileReader *reader) { return Z3DS_CreateDecoder(reader); }')
s=s.replace('SoundStream *OpenStream(FileReader *reader, int flags)','SoundStream *OpenReaderStream(FileReader *reader, int flags)')
s=s.replace('struct NDSPSample\n{','struct NDSPSample\n{\n\tu32 Bytes, LastUse;')
s=s.replace('static NDSPSoundRenderer *TheRenderer;','static NDSPSoundRenderer *TheRenderer;\nstatic unsigned SampleBytes;\nstatic const unsigned SampleBudget=1536*1024;')
s=s.replace('\t\tlinearFree(s->PCM);','\t\tSampleBytes-=s->Bytes;\n\t\tlinearFree(s->PCM);')
s=s.replace('\t\tif (frames > 0)\n\t\t{','\t\tif (frames > 0)\n\t\t{\n\t\t\tif(frames > (768*1024/2)*(rate>=32000?2:1)) { delete decoder; return retval; }')
s=s.replace('\t\tu32 outframes = frames / step;','\t\tu32 outframes = frames / step;\n\t\tif(outframes*2>768*1024) return retval;\n\t\twhile(SampleBytes+outframes*2>SampleBudget) { if(!EvictSample()) return retval; }')
s=s.replace('\t\ts->PCM = pcm;','\t\ts->PCM = pcm;\n\t\ts->Bytes=outframes*2; s->LastUse=osGetTime(); SampleBytes+=s->Bytes;')
s=s.replace('\t\tvo.Sample = s;','\t\ts->LastUse=osGetTime();\n\t\tvo.Sample = s;')
s=s.replace('\tSoundHandle MakeSample(const BYTE *data,', '''
    bool EvictSample() {
        sfxinfo_t *candidate=NULL; u32 oldest=0;
        for(unsigned i=0;i<S_sfx.Size();++i) {
            NDSPSample *sample=(NDSPSample*)S_sfx[i].data.data;
            if(!sample) continue;
            bool active=false;
            for(int v=0;v<SFX_VOICES;++v) if(Voices[v].Active && Voices[v].Sample==sample) active=true;
            u32 age=(u32)osGetTime()-sample->LastUse;
            if(!active && (!candidate || age>oldest)) { candidate=&S_sfx[i]; oldest=age; }
        }
        if(!candidate) return false;
        S_UnloadSound(candidate); return true;
    }
\tSoundHandle MakeSample(const BYTE *data,''')
marker='\tSoundStream *OpenReaderStream(FileReader *reader, int flags)'
spool='''
    SoundStream *OpenStream(const char *source,int flags,int offset,int length) {
        char path[128];
        if(offset<0) {
            if(length<=0) return NULL;
            BYTE hash[16]; MD5Context md5; md5.Update((const BYTE*)source,length); md5.Final(hash);
            strcpy(path,"sdmc:/3ds/zandronum/audio-cache/");
            unsigned end=strlen(path);
            for(unsigned i=0;i<16;++i) sprintf(path+end+i*2,"%02x",hash[i]);
            strcat(path,".bin"); mkdir("sdmc:/3ds/zandronum/audio-cache",0777);
            struct stat st;
            if(stat(path,&st) || st.st_size!=length) {
                FILE *output=fopen(path,"wb"); if(!output) return NULL;
                bool ok=fwrite(source,1,length,output)==(size_t)length;
                ok=fclose(output)==0 && ok; if(!ok) return NULL;
            }
            source=path; offset=0;
        }
        FILE *file=fopen(source,"rb"); if(!file) return NULL;
        if(fseek(file,offset,SEEK_SET)) { fclose(file); return NULL; }
        class OwnedReader:public FileReader {
        public: OwnedReader(FILE*f,long n):FileReader(f,n) { CloseOnDestruct=true; }
        };
        return OpenReaderStream(new OwnedReader(file,length),flags);
    }
'''
assert marker in s
s=s.replace(marker,spool+marker)
(out/'ndspsound.cpp').write_text(s)
print('Imported NDSP/CSND streaming renderer, decoder and DSP extraction source')

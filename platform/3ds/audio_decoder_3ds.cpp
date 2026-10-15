#include "audio_decoder_3ds.h"
#include <stdint.h>
#include "m_argv.h"
#include "i_system.h"
#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"
#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"

SoundDecoder *I3DS_CreateVorbisDecoder();
SoundDecoder *I3DS_CreateWavDecoder();
static size_t ReadPCM(void *p,void *out,size_t bytes) { long n=((FileReader*)p)->Read(out,bytes); return n>0?n:0; }
static drmp3_bool32 SeekMP3(void *p,int offset,drmp3_seek_origin origin) { return ((FileReader*)p)->Seek(offset,origin==DRMP3_SEEK_SET?SEEK_SET:origin==DRMP3_SEEK_END?SEEK_END:SEEK_CUR)==0; }
static drmp3_bool32 TellMP3(void *p,drmp3_int64 *out) { *out=((FileReader*)p)->Tell(); return *out>=0; }
static drflac_bool32 SeekFLAC(void *p,int offset,drflac_seek_origin origin) { return ((FileReader*)p)->Seek(offset,origin==DRFLAC_SEEK_SET?SEEK_SET:origin==DRFLAC_SEEK_END?SEEK_END:SEEK_CUR)==0; }
static drflac_bool32 TellFLAC(void *p,drflac_int64 *out) { *out=((FileReader*)p)->Tell(); return *out>=0; }
class MP3Decoder : public SoundDecoder {
    drmp3 file; bool opened=false;
public:
    ~MP3Decoder() { if(opened) drmp3_uninit(&file); }
    bool open(FileReader *r) { opened=drmp3_init(&file,ReadPCM,SeekMP3,TellMP3,NULL,r,NULL); return opened && file.channels<=2; }
    void getInfo(int *rate,ChannelConfig *channels,SampleType *type) { *rate=file.sampleRate; *channels=file.channels==1?ChannelConfig_Mono:ChannelConfig_Stereo; *type=SampleType_Int16; }
    size_t read(char *out,size_t bytes) { return drmp3_read_pcm_frames_s16(&file,bytes/(2*file.channels),(drmp3_int16*)out)*2*file.channels; }
    bool seek(size_t ms) { return drmp3_seek_to_pcm_frame(&file,(uint64_t)ms*file.sampleRate/1000); }
    size_t getSampleOffset() { return file.currentPCMFrame; }
    size_t getSampleLength() { uint64_t n=drmp3_get_pcm_frame_count(&file); drmp3_seek_to_pcm_frame(&file,0); return n; }
};
class FLACDecoder : public SoundDecoder {
    drflac *file=NULL;
public:
    ~FLACDecoder() { if(file) drflac_close(file); }
    bool open(FileReader *r) { file=drflac_open(ReadPCM,SeekFLAC,TellFLAC,r,NULL); return file && file->channels<=2; }
    void getInfo(int *rate,ChannelConfig *channels,SampleType *type) { *rate=file->sampleRate; *channels=file->channels==1?ChannelConfig_Mono:ChannelConfig_Stereo; *type=SampleType_Int16; }
    size_t read(char *out,size_t bytes) { return drflac_read_pcm_frames_s16(file,bytes/(2*file->channels),(drflac_int16*)out)*2*file->channels; }
    bool seek(size_t ms) { return drflac_seek_to_pcm_frame(file,(uint64_t)ms*file->sampleRate/1000); }
    size_t getSampleOffset() { return file->currentPCMFrame; }
    size_t getSampleLength() { return file->totalPCMFrameCount; }
};
TArray<char> SoundDecoder::readAll() {
    TArray<char> data; char buffer[4096];
    // Unknown-length effects are bounded too; compressed music uses streaming.
    for(;;) { size_t got=read(buffer,sizeof(buffer)); if(!got) break;
        if(data.Size()+got>3*1024*1024) { data.Clear(); break; }
        unsigned at=data.Size(); data.Resize(at+got); memcpy(&data[at],buffer,got);
    }
    return data;
}
SoundDecoder *Z3DS_CreateDecoder(FileReader *reader) {
    char magic[4]; reader->Seek(0,SEEK_SET); long got=reader->Read(magic,4); reader->Seek(0,SEEK_SET);
    if(got!=4) return NULL;
    SoundDecoder *decoder= !memcmp(magic,"OggS",4)?I3DS_CreateVorbisDecoder():!memcmp(magic,"RIFF",4)?I3DS_CreateWavDecoder():!memcmp(magic,"fLaC",4)?static_cast<SoundDecoder*>(new FLACDecoder):static_cast<SoundDecoder*>(new MP3Decoder);
    if(decoder->open(reader)) {
        if(Args && Args->CheckParm("-3ds-test-audio")) {
            int rate; ChannelConfig channels; SampleType type; decoder->getInfo(&rate,&channels,&type);
            Printf("3DS AUDIO DECODER opened %s, rate=%d channels=%d\n",!memcmp(magic,"OggS",4)?"Vorbis":!memcmp(magic,"RIFF",4)?"WAV":!memcmp(magic,"fLaC",4)?"FLAC":"MP3",rate,channels==ChannelConfig_Stereo?2:1);
        }
        return decoder;
    }
    delete decoder; reader->Seek(0,SEEK_SET); return NULL;
}

#pragma once
#include "files.h"
#include "tarray.h"
enum SampleType { SampleType_UInt8, SampleType_Int16 };
enum ChannelConfig { ChannelConfig_Mono, ChannelConfig_Stereo };
struct SoundDecoder {
    virtual ~SoundDecoder() {}
    virtual bool open(FileReader *)=0;
    virtual void getInfo(int *,ChannelConfig *,SampleType *)=0;
    virtual size_t read(char *,size_t)=0;
    virtual bool seek(size_t)=0;
    virtual size_t getSampleOffset()=0;
    virtual size_t getSampleLength()=0;
    TArray<char> readAll();
};
SoundDecoder *Z3DS_CreateDecoder(FileReader *);

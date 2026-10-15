#include <3ds.h>
#include <malloc.h>
#include "textures/textures.h"
#include "dobject.h"
#include "i_system.h"
#include "i_3ds.h"
#include "doomstat.h"
#include "doomdef.h"
#include "c_cvars.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
struct DumbAllocation { unsigned magic, bytes, pc, reserved; };
struct DumbSite { unsigned pc, bytes, peak; };
static struct DumbTrace {
    LightLock lock; DumbSite sites[64];
    DumbTrace() { LightLock_Init(&lock); memset(sites,0,sizeof(sites)); }
} dumbTrace;
static void TrackDumb(unsigned pc,unsigned bytes,bool add)
{
    LightLock_Lock(&dumbTrace.lock);
    unsigned i=0;
    for(;i<63 && dumbTrace.sites[i].pc && dumbTrace.sites[i].pc!=pc;++i) {}
    DumbSite &site=dumbTrace.sites[i];
    if(!site.pc) site.pc=pc;
    if(add) site.bytes+=bytes; else site.bytes-=bytes;
    if(site.peak<site.bytes) site.peak=site.bytes;
    LightLock_Unlock(&dumbTrace.lock);
}
static void *AllocDumb(size_t bytes,unsigned pc,bool zero)
{
    if(bytes>(size_t)-1-sizeof(DumbAllocation)) return NULL;
    DumbAllocation *p=(DumbAllocation*)malloc(sizeof(DumbAllocation)+bytes);
    if(!p) return NULL;
    *p={0xD3D5A110,(unsigned)bytes,pc,0};
    TrackDumb(pc,bytes,true);
    if(zero) memset(p+1,0,bytes);
    return p+1;
}
extern "C" void *Z3DS_DumbMalloc(size_t bytes) { return AllocDumb(bytes,(unsigned)__builtin_return_address(0),false); }
extern "C" void *Z3DS_DumbCalloc(size_t n,size_t bytes) {
    if(bytes && n>(size_t)-1/bytes) return NULL;
    return AllocDumb(n*bytes,(unsigned)__builtin_return_address(0),true);
}
extern "C" void Z3DS_DumbFree(void *memory) {
    if(!memory) return;
    DumbAllocation *p=(DumbAllocation*)memory-1;
    if(p->magic!=0xD3D5A110) { free(memory); return; }
    TrackDumb(p->pc,p->bytes,false); p->magic=0; free(p);
}
extern "C" void *Z3DS_DumbRealloc(void *memory,size_t bytes) {
    unsigned pc=(unsigned)__builtin_return_address(0);
    if(!memory) return AllocDumb(bytes,pc,false);
    DumbAllocation *p=(DumbAllocation*)memory-1;
    if(p->magic!=0xD3D5A110) return realloc(memory,bytes);
    if(!bytes) { Z3DS_DumbFree(memory); return NULL; }
    if(bytes>(size_t)-1-sizeof(DumbAllocation)) return NULL;
    unsigned oldPC=p->pc,oldBytes=p->bytes;
    p=(DumbAllocation*)realloc(p,sizeof(DumbAllocation)+bytes);
    if(!p) return NULL;
    TrackDumb(oldPC,oldBytes,false); TrackDumb(pc,bytes,true);
    *p={0xD3D5A110,(unsigned)bytes,pc,0}; return p+1;
}
CVAR(Bool, native_resource_trim_3ds, true, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
// Read the clock once per input/frame, not once per rendered texture column.
CVAR(Bool, native_texture_clock_3ds, true, 0)
static unsigned lastSweep, levelStarted, textureClock, textureTouches;
static unsigned frameStart, drawStart, reportStart, frames, tickMS, drawMS, collectMS;
static unsigned maxFrameMS, maxCollectMS;
static unsigned inputMS, inputCalls, presentMS, viewMS;
void I3DS_RecordCost(unsigned kind, unsigned ms)
{
    if(kind==0) { inputMS+=ms; ++inputCalls; }
    if(kind==1) presentMS+=ms;
    if(kind==2) viewMS+=ms;
}
void I3DS_BeginFrame()
{
    frameStart=textureClock=I_MSTime();
    drawStart=0;
    if(!reportStart) reportStart=frameStart;
}
void I3DS_BeginDraw() { drawStart=textureClock=I_MSTime(); }
FTexture *Z3DS_TouchTexture(FTexture *texture)
{
    if(texture) { ++textureTouches; texture->NativeLastUse=native_texture_clock_3ds ? textureClock : I_MSTime(); }
    return texture;
}
void I3DS_MemoryTick()
{
    textureClock=I_MSTime();
}
static void TrimAfterFrame()
{
    static bool pressure;
    unsigned now=textureClock=I_MSTime();
    // Age remains real milliseconds even though column reads share a clock.
    // Static menus are excluded; game and animated-title maps get startup grace.
    if(gamestate!=GS_LEVEL && gamestate!=GS_TITLELEVEL) { levelStarted=0; return; }
    if(!levelStarted) levelStarted=now;
    if(!native_resource_trim_3ds || now-levelStarted<10000) return;
    // Heap accounting and eviction happen once per completed frame. Input is
    // also polled inside rendering, where temporary texture pointers are live.
    if(now-lastSweep<(pressure?500u:2000u)) return;
    unsigned used=mallinfo().uordblks, capacity=envGetHeapSize();
    pressure=used+20*1024*1024>capacity;
    lastSweep=now;
    unsigned before=used;
    unsigned age=pressure?2000:15000;
    TexMan.PurgeNativePixels(now,age);
    if(pressure) { extern unsigned Z3DS_PurgeResourceCache(); Z3DS_PurgeResourceCache(); }
    unsigned after=mallinfo().uordblks;
    if(I3DS_Diagnostics() && before>after+256*1024) Printf("3DS memory: reclaimed %u KiB; heap %u/%u KiB\n",(before-after)/1024,after/1024,capacity/1024);
}

void I3DS_CollectAfterFrame()
{
    // Actors and temporary render references are settled here, not mid-input.
    static unsigned lastCollection;
    unsigned now=I_MSTime();
    if(drawStart) { tickMS+=drawStart-frameStart; drawMS+=now-drawStart; }
    TrimAfterFrame();
    // The engine's incremental collector spreads work over frames. A forced
    // full collection every two seconds stopped camera movement periodically.
    if(GC::State!=GC::GCS_Pause || now-lastCollection>=2000) {
        if(GC::State==GC::GCS_Pause) lastCollection=now;
        // Keep inactive/destroyed actors collectible even when allocation is
        // too low to reach the engine threshold, but do only one bounded step.
        GC::StartCollection();
    }
    GC::CheckGC();
    unsigned end=I_MSTime();
    collectMS+=end-now; ++frames;
    if(end-frameStart>maxFrameMS) maxFrameMS=end-frameStart;
    if(end-now>maxCollectMS) maxCollectMS=end-now;
    if(I3DS_Diagnostics() && end-reportStart>=10000) {
        FILE *log=fopen("sdmc:/3ds/zandronum/performance.txt","a");
        if(log) {
            unsigned objects=0;
            for(DObject *object=GC::Root; object; object=object->ObjNext) ++objects;
            fprintf(log,"state=%d frames=%u elapsed=%u ms tick=%u draw=%u maintenance=%u touches=%u heap=%u KiB gametic=%d input=%u/%u present=%u view=%u gcbytes=%u objects=%u\n",(int)gamestate,frames,end-reportStart,tickMS,drawMS,collectMS,textureTouches,(unsigned)mallinfo().uordblks/1024,gametic,inputMS,inputCalls,presentMS,viewMS,(unsigned)GC::AllocBytes/1024,objects);
            extern unsigned Z3DS_PNGResidentBytes();
            fprintf(log,"PNG resident: %u KiB\n",Z3DS_PNGResidentBytes()/1024);
            fprintf(log,"Frame max: %u ms; maintenance max: %u ms\n",maxFrameMS,maxCollectMS);
            LightLock_Lock(&dumbTrace.lock);
            for(unsigned i=0;i<64;++i) if(dumbTrace.sites[i].bytes>=32768) {
                DumbSite &s=dumbTrace.sites[i];
                fprintf(log,"DUMB pc=%08x live=%u KiB peak=%u KiB\n",s.pc,s.bytes/1024,s.peak/1024);
            }
            LightLock_Unlock(&dumbTrace.lock);
            fclose(log);
        }
        reportStart=end; frames=tickMS=drawMS=collectMS=textureTouches=0;
        inputMS=inputCalls=presentMS=viewMS=0;
        maxFrameMS=maxCollectMS=0;
    }
}

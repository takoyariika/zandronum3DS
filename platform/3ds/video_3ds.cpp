// Zandronum software framebuffer presented on the 3DS top LCD.
// Native 400x240 top LCD: render the wider view instead of stretching 320x240.
extern "C" {
#include <3ds/types.h>
#include <3ds/gfx.h>
#include <3ds/services/gspgpu.h>
}
#include <cmath>
#include <cstring>
#include <cstdio>
#include <malloc.h>
#include "doomtype.h"
#include "v_video.h"
#include "v_palette.h"
#include "hardware.h"
#include "i_3ds.h"
#include "i_system.h"
#include "doomstat.h"
#include "doomdef.h"

class Z3DSFrameBuffer : public DFrameBuffer
{
    DECLARE_CLASS(Z3DSFrameBuffer, DFrameBuffer)
public:
    Z3DSFrameBuffer(int w, int h) : DFrameBuffer(w, h), flash(0), amount(0), gamma(1.f), updatePending(false)
    {
        memcpy(palette, GPalette.BaseColors, sizeof(palette));
        UpdatePalette();
    }
    bool Lock(bool buffered) { return DSimpleCanvas::Lock(); }
    bool Relock() { return DSimpleCanvas::Lock(); }
    void Unlock()
    {
        if (updatePending && LockCount == 1) Update();
        else DSimpleCanvas::Unlock();
    }
    PalEntry *GetPalette() { return palette; }
    void GetFlashedPalette(PalEntry p[256]) { memcpy(p, adjusted, sizeof(adjusted)); }
    void UpdatePalette()
    {
        for (int i = 0; i < 256; ++i)
        {
            int r = (palette[i].r * (256-amount) + flash.r * amount) >> 8;
            int g = (palette[i].g * (256-amount) + flash.g * amount) >> 8;
            int b = (palette[i].b * (256-amount) + flash.b * amount) >> 8;
            adjusted[i] = PalEntry(curve(r), curve(g), curve(b));
        }
    }
    bool SetGamma(float v) { gamma = v > 0.f ? v : 1.f; UpdatePalette(); return true; }
    bool SetFlash(PalEntry rgb, int v)
    {
        flash = rgb; amount = v < 0 ? 0 : v > 256 ? 256 : v;
        UpdatePalette(); return true;
    }
    void GetFlash(PalEntry &rgb, int &v) { rgb = flash; v = amount; }
    int GetPageCount() { return 2; }
    bool IsFullscreen() { return true; }
    void Update()
    {
        if (LockCount != 1)
        {
            if (LockCount > 0) { updatePending = true; --LockCount; }
            return;
        }
        unsigned presentStart=I_MSTime();
        if (!I3DS_PumpEvents())
        {
            DSimpleCanvas::Unlock(); updatePending=false;
            return;
        }
        DrawRateStuff();
        I3DS_EndLoadingDisplay();
        u16 lcdWidth, lcdHeight;
        u8 *dest = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &lcdWidth, &lcdHeight);
        static unsigned lastVideoLog, videoReports;
        static int videoState=-1;
        if(videoState!=(int)gamestate) { videoState=(int)gamestate; videoReports=0; lastVideoLog=0; }
        // libctru's framebuffer is rotated: 240 rows per column, BGR24.
        if (lcdWidth == 240 && lcdHeight == 400)
        {
            for (int x = 0; x < Width; ++x)
                for (int y = 0; y < Height; ++y)
                {
                    const PalEntry &c = adjusted[Buffer[y * Pitch + x]];
                    u8 *p = dest + (x * 240 + (239-y)) * 3;
                    p[0] = c.b; p[1] = c.g; p[2] = c.r;
                }
            I3DS_DrawBottom();
            gfxFlushBuffers();
            gfxSwapBuffers();
            gspWaitForVBlank();
            unsigned now=I_MSTime();
            if(I3DS_Diagnostics() && videoReports<4 && now-lastVideoLog>=5000) {
                lastVideoLog=now; ++videoReports;
                unsigned visible=0;
                for(unsigned p=0;p<240*400*3;p+=3) if(dest[p] || dest[p+1] || dest[p+2]) ++visible;
                FILE *log=fopen("sdmc:/3ds/zandronum/video.txt","a");
                if(log) { fprintf(log,"frame %u: state=%d canvas=%dx%d pitch=%d LCD=%ux%u nonblack=%u/96000 heap=%u KiB\n",videoReports,(int)gamestate,Width,Height,Pitch,lcdWidth,lcdHeight,visible,(unsigned)mallinfo().uordblks/1024); fclose(log); }
                FILE *dump=fopen("sdmc:/3ds/zandronum/video-top.raw","wb");
                if(dump) { u16 dims[]={lcdWidth,lcdHeight}; fwrite(dims,sizeof(dims),1,dump); fwrite(dest,3,(size_t)lcdWidth*lcdHeight,dump); fclose(dump); }
            }
        }
        DSimpleCanvas::Unlock();
        updatePending = false;
        extern void I3DS_RecordCost(unsigned,unsigned);
        I3DS_RecordCost(1,I_MSTime()-presentStart);
    }
private:
    Z3DSFrameBuffer() : flash(0), amount(0), gamma(1.f), updatePending(false) {}
    BYTE curve(int n) const { return BYTE(255.f * std::pow(n / 255.f, 1.f / gamma) + .5f); }
    PalEntry palette[256], adjusted[256], flash;
    int amount;
    float gamma;
    bool updatePending;
};
IMPLEMENT_CLASS(Z3DSFrameBuffer)

class Z3DSVideo : public IVideo
{
public:
    Z3DSVideo() : available(false) {}
    EDisplayType GetDisplayType() { return DISPLAY_FullscreenOnly; }
    void SetWindowedScale(float) {}
    DFrameBuffer *CreateFrameBuffer(int w, int h, bool fs, DFrameBuffer *old)
    {
        if (old && old->GetWidth() == 400 && old->GetHeight() == 240) return old;
        if (old)
        {
            old->ObjectFlags |= OF_YesReallyDelete;
            if (screen == old) screen = NULL;
            delete old;
        }
        return new Z3DSFrameBuffer(400, 240);
    }
    void StartModeIterator(int bits, bool) { available = bits == 8; }
    bool NextMode(int *w, int *h, bool *letterbox)
    {
        if (!available) return false;
        available = false; *w = 400; *h = 240;
        if (letterbox) *letterbox = false;
        return true;
    }
private:
    bool available;
};
IVideo *Z3DS_CreateVideo() { return new Z3DSVideo; }

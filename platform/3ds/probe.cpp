// SDK / SD / LCD diagnostic. This does not contain or run the Doom engine.
#include <3ds.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <malloc.h>

extern "C" { u32 __stacksize__ = 128 * 1024; }

static unsigned read16(const unsigned char *p) { return p[0] | (p[1] << 8); }
static unsigned read32(const unsigned char *p) { return read16(p) | (unsigned(read16(p+2)) << 16); }

static void inspectPK3()
{
    const char *path = "sdmc:/3ds/zandronum/BYOCv2.0.pk3";
    FILE *f = fopen(path, "rb");
    if (!f) { printf("PK3 missing:\n%s\n", path); return; }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    if (size < 22) { printf("PK3: truncated\n"); fclose(f); return; }
    size_t tailSize = size < 65557 ? size : 65557;
    unsigned char *tail = static_cast<unsigned char *>(malloc(tailSize));
    if (!tail) { printf("PK3: no buffer memory\n"); fclose(f); return; }
    fseek(f, size-tailSize, SEEK_SET);
    if (fread(tail, 1, tailSize, f) != tailSize) { printf("PK3: SD read failed\n"); free(tail); fclose(f); return; }
    bool found = false;
    for (int i = int(tailSize)-22; i >= 0; --i)
    {
        const unsigned char *e = tail+i;
        if (read32(e) != 0x06054b50 || i+22+read16(e+20) != tailSize) continue;
        unsigned entries = read16(e+10), dirSize = read32(e+12), offset = read32(e+16);
        if (read16(e+4) || read16(e+6) || read16(e+8) != entries ||
            entries == 65535 || offset == 0xffffffff || dirSize == 0xffffffff ||
            (unsigned long long)offset+dirSize > (unsigned long long)(size-tailSize+i))
            break;
        printf("PK3: %ld bytes\nZIP directory: %u entries\n", size, entries);
        printf("Index disk size: %u KiB\n", dirSize/1024);
        found = true; break;
    }
    if (!found) printf("PK3: invalid/unsupported ZIP directory\n");
    free(tail); fclose(f);
}

int main()
{
    osSetSpeedupEnable(true);
    gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL);
    bool new3ds = false;
    APT_CheckNew3DS(&new3ds);
    printf("Zandronum 3DS platform probe\n");
    printf("Diagnostic only - no game engine\n\n");
    printf("New 3DS: %s\n", new3ds ? "yes" : "no");
    printf("int / pointer: %u / %u bits\n", unsigned(sizeof(int)*8), unsigned(sizeof(void*)*8));
    printf("Linear free: %u KiB\n", unsigned(linearSpaceFree()/1024));
    FILE *iwad = fopen("sdmc:/3ds/zandronum/DOOM2.WAD", "rb");
    if (iwad)
    {
        unsigned char head[12];
        bool valid = fread(head, 1, sizeof(head), iwad) == sizeof(head) && !memcmp(head, "IWAD", 4);
        printf("DOOM2.WAD header: %s\n", valid ? "IWAD" : "invalid");
        fclose(iwad);
    }
    else printf("DOOM2.WAD: missing\n");
    inspectPK3();
    printf("\nSTART: exit\n");
    while (aptMainLoop())
    {
        hidScanInput();
        if (hidKeysDown() & KEY_START) break;
        u16 w, h;
        u8 *fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &w, &h);
        for (unsigned x = 0; x < h; ++x)
            for (unsigned y = 0; y < w; ++y)
            {
                u8 *p = fb + (x*w + (w-1-y))*3;
                p[0] = x >= 40 && x < 360 ? u8(y) : 0;
                p[1] = x >= 40 && x < 360 ? u8(x-40) : 0;
                p[2] = x >= 40 && x < 360 ? 180 : 0;
            }
        gfxFlushBuffers(); gfxSwapBuffers(); gspWaitForVBlank();
    }
    gfxExit();
    return 0;
}

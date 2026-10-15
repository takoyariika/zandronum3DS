#pragma once
#include <3ds.h>
#include <string>
#include <algorithm>
#include <cstdio>

// Small framebuffer UI: no GPU context or asset heap survives engine startup.
namespace LauncherUI {
inline u16 RGB(unsigned r,unsigned g,unsigned b) { return ((r>>3)<<11)|((g>>2)<<5)|(b>>3); }
const u16 background=RGB(13,19,29), panel=RGB(24,34,49), selected=RGB(34,68,85);
const u16 ink=RGB(232,240,247), muted=RGB(142,161,181), accent=RGB(65,219,178), warning=RGB(255,194,100);
inline void Rect(gfxScreen_t s,int x,int y,int w,int h,u16 color) {
    u16 fw,fh; u16 *fb=(u16*)gfxGetFramebuffer(s,GFX_LEFT,&fw,&fh);
    int width=s==GFX_TOP?400:320;
    for(int xx=std::max(0,x);xx<std::min(width,x+w);++xx)
        for(int yy=std::max(0,y);yy<std::min(240,y+h);++yy) fb[xx*240+239-yy]=color;
}
inline void Text(gfxScreen_t s,int x,int y,const std::string &text,u16 color=ink,int maxChars=48) {
    const u8 *font=consoleGetDefault()->font.gfx; int n=0;
    for(unsigned char ch:text) {
        if(ch==0x1c) continue;
        if(n++>=maxChars) break;
        if(ch<32 || ch>=128) ch='?';
        for(int row=0;row<8;++row) for(int col=0;col<8;++col)
            if(font[ch*8+row] & (0x80>>col)) Rect(s,x+col,y+row,1,1,color);
        x+=8;
    }
}
inline void Wrapped(gfxScreen_t s,int x,int y,const std::string &text,int columns,int rows,u16 color=ink) {
    for(int row=0;row<rows && size_t(row*columns)<text.size();++row)
        Text(s,x,y+row*12,text.substr(row*columns,columns),color,columns);
}
inline void Begin(const std::string &title,const std::string &subtitle) {
    Rect(GFX_TOP,0,0,400,240,background); Rect(GFX_BOTTOM,0,0,320,240,background);
    Rect(GFX_TOP,0,0,400,39,panel); Rect(GFX_TOP,0,0,4,39,accent);
    Text(GFX_TOP,14,9,title,ink); Text(GFX_TOP,14,24,subtitle,muted);
    Rect(GFX_BOTTOM,0,0,320,3,accent);
}
inline void Card(unsigned row,const std::string &title,const std::string &subtitle,bool active,const std::string &badge="") {
    int y=46+row*28;
    Rect(GFX_TOP,8,y,384,25,active?selected:panel);
    if(active) Rect(GFX_TOP,8,y,3,25,accent);
    Text(GFX_TOP,18,y+4,title,active?ink:muted,badge.empty()?45:38);
    Text(GFX_TOP,18,y+15,subtitle,muted,45);
    if(!badge.empty()) Text(GFX_TOP,332,y+4,badge,accent,7);
}
inline void Button(int column,int row,const std::string &label,u16 color=panel) {
    int x=8+column*156,y=170+row*33;
    Rect(GFX_BOTTOM,x,y,148,28,color); Text(GFX_BOTTOM,x+8,y+10,label,ink,16);
}
inline void Present() { gfxFlushBuffers(); gspWaitForVBlank(); }
inline void Dump(const char *prefix) {
    for(gfxScreen_t screen:{GFX_TOP,GFX_BOTTOM}) {
        std::string path=std::string("sdmc:/3ds/zandronum/")+prefix+(screen==GFX_TOP?"-top.rgb565":"-bottom.rgb565");
        FILE *file=fopen(path.c_str(),"wb");
        if(file) { u16 dims[]={u16(screen==GFX_TOP?400:320),240}; fwrite(dims,2,2,file); fwrite(gfxGetFramebuffer(screen,GFX_LEFT,NULL,NULL),2,dims[0]*dims[1],file); fclose(file); }
    }
}
inline u32 TouchButtons(u32 keys,u32 a,u32 b,u32 c,u32 d) {
    if(!(keys & KEY_TOUCH)) return 0;
    touchPosition p; hidTouchRead(&p);
    if(p.px<8 || p.px>=312 || p.py<170 || p.py>=231) return 0;
    int column=p.px>=164,row=p.py>=203;
    return row?(column?d:c):(column?b:a);
}
inline void Message(const std::string &title,const std::string &message) {
    Begin(title,"ZANDRONUM 3.2.1 / NEW 3DS");
    Wrapped(GFX_TOP,16,58,message,46,12,muted); Text(GFX_BOTTOM,16,24,title,accent,36);
    Text(GFX_BOTTOM,16,50,"B: cancel",muted); Present();
}
}

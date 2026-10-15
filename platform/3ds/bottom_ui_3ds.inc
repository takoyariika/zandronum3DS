// Included once by input_3ds.cpp. Bounded storage; no disk IO or per-frame log allocation.
#include "launcher_ui_3ds.h"
#include <cstring>
namespace {
char bottomLines[128][39]={};
unsigned bottomLine=0,bottomColumn=0,bottomCount=1,bottomPage=0;
bool bottomScores=false,bottomDirty=true,bottomSlow=false;
enum BottomOwner { NoTouch, LookTouch, ViewTouch, SlowTouch, PageTouch };
BottomOwner bottomOwner=NoTouch;
const int bottomSpeedKey=0x7ffe; // Dedicated source; releasing never clears other speed keys.
void BottomSlow(bool slow) {
    if(slow==bottomSlow) return;
    bottomSlow=slow; bottomDirty=true;
    if(slow) Button_Speed.PressKey(bottomSpeedKey);
    else Button_Speed.ReleaseKey(bottomSpeedKey);
}
std::string BottomName(const char *name) {
    std::string result;
    for(const unsigned char *p=(const unsigned char*)name;*p && result.size()<16;++p) {
        if(*p==0x1c) {
            if(p[1]=='[') { ++p; while(p[1] && p[1]!=']') ++p; if(p[1]) ++p; }
            else if(p[1]) ++p;
        } else if(*p>=32 && *p<127) result+=(char)*p;
    }
    return result;
}
}
void I3DS_BottomLog(const char *text) {
    for(const unsigned char *p=(const unsigned char*)text;*p;++p) {
        if(*p=='\r') continue;
        if(*p=='\n' || bottomColumn==38) {
            bottomLine=(bottomLine+1)%128; bottomColumn=0;
            bottomLines[bottomLine][0]=0;
            if(bottomCount<128) ++bottomCount;
            if(*p=='\n') continue;
        }
        if(*p>=32) {
            bottomLines[bottomLine][bottomColumn++]=*p<127?*p:'?';
            bottomLines[bottomLine][bottomColumn]=0;
        }
    }
    bottomDirty=true;
}
static void I3DS_ResetBottomTouch() { BottomSlow(false); bottomOwner=NoTouch; }
void I3DS_ResetBottomScreen() {
    I3DS_ResetBottomTouch(); bottomScores=false; bottomPage=0;
    bottomLine=bottomColumn=0; bottomCount=1;
    memset(bottomLines,0,sizeof(bottomLines)); bottomDirty=true;
}
static bool I3DS_BottomTouch(bool down, const touchPosition *injected=NULL) {
    bool enabled=(gamestate==GS_LEVEL || gamestate==GS_INTERMISSION || gamestate==GS_TITLELEVEL || gamestate==GS_DEMOSCREEN) && menuactive==MENU_Off;
    if(!down) { I3DS_ResetBottomTouch(); return false; }
    touchPosition p; if(injected) p=*injected; else hidTouchRead(&p);
    if(!enabled) { BottomSlow(false); bottomOwner=ViewTouch; return true; }
    if(bottomOwner==NoTouch) {
        if(p.py<40 && p.px<156) {
            bottomOwner=ViewTouch; bottomScores=!bottomScores; bottomDirty=true;
        } else if(p.py<40 && p.px>=164) bottomOwner=SlowTouch;
        else if(bottomScores && p.py>=216) { bottomOwner=PageTouch; ++bottomPage; bottomDirty=true; }
        else bottomOwner=LookTouch;
    }
    BottomSlow(bottomOwner==SlowTouch && p.py<40 && p.px>=164 && gamestate==GS_LEVEL && cl_run);
    return bottomOwner!=LookTouch;
}
static void I3DS_TestBottom() {
    static unsigned started,stage;
    if(!started) started=I_MSTime();
    unsigned next=(I_MSTime()-started)/700;
    if(next==stage || next>9) return;
    stage=next;
    touchPosition right={240,18},left={50,18},outside={240,95};
    auto require=[](bool ok,const char *message) { if(!ok) I_FatalError("Bottom controls regression: %s",message); };
    switch(stage) {
    case 1:
        require(cl_run,"default autorun");
        for(unsigned i=0;i<200;++i) I3DS_BottomLog("Recent game event: bounded log history\n");
        require(bottomCount==128,"bounded log ring");
        I3DS_ResetBottomTouch();
        require(I3DS_BottomTouch(true,&right) && Button_Speed.bDown,"hold to walk");
        I3DS_DrawBottom(); LauncherUI::Dump("ui-bottom-walk");
        break;
    case 2:
        require(I3DS_BottomTouch(true,&outside) && !Button_Speed.bDown,"slide out releases without camera drag"); break;
    case 3:
        I3DS_BottomTouch(false);
        I3DS_BottomTouch(true,&left);
        require(bottomScores,"toggle scoreboard");
        I3DS_BottomTouch(true,&left);
        require(bottomScores,"held toggle must not repeat");
        I3DS_BottomTouch(true,&right);
        require(!Button_Speed.bDown,"view gesture must not become walk");
        I3DS_DrawBottom(); LauncherUI::Dump("ui-bottom-scores");
        break;
    case 4:
        I3DS_BottomTouch(false); I3DS_BottomTouch(true,&right);
        I3DS_ResetBottomTouch();
        require(!Button_Speed.bDown,"resume reset releases walk"); break;
    case 5: {
        I3DS_BottomTouch(true,&right);
        auto saved=menuactive; menuactive=MENU_On;
        I3DS_BottomTouch(true,&right); menuactive=saved;
        require(!Button_Speed.bDown,"opening menu releases walk");
        I3DS_BottomTouch(false);
        I3DS_BottomTouch(true,&outside);
        require(!I3DS_BottomTouch(true,&right) && !Button_Speed.bDown,"camera drag cannot press walk");
        break;
    }
    case 6:
        I3DS_BottomTouch(false); I3DS_BottomTouch(true,&left);
        require(!bottomScores,"toggle back to log");
        I3DS_DrawBottom(); LauncherUI::Dump("ui-bottom-log"); break;
    case 7:
        I3DS_ResetBottomTouch();
        Button_Speed.PressKey(1234);
        I3DS_BottomTouch(true,&right); I3DS_BottomTouch(false);
        require(Button_Speed.bDown,"release preserves other speed input");
        Button_Speed.ReleaseKey(1234);
        require(!Button_Speed.bDown,"walk fully released");
        Printf("3DS BOTTOM TEST PASS: autorun, walk hold/release, slide, menu/resume reset, gesture isolation, view toggle and bounded logs\n");
        break;
    case 9:
        Printf("3DS QUIT TEST: normal menu exit path\n"); ST_Endoom(); break;
    }
}
void I3DS_DrawBottom() {
    using namespace LauncherUI;
    static unsigned lastDraw;
    unsigned now=I_MSTime();
    if(!bottomDirty && (!bottomScores || now-lastDraw<200)) return;
    // At most five log refreshes per second; touch feedback draws immediately.
    if(bottomDirty && bottomOwner==NoTouch && now-lastDraw<200) return;
    bottomDirty=false; lastDraw=now;
    Rect(GFX_BOTTOM,0,0,320,240,background);
    Rect(GFX_BOTTOM,8,6,144,30,bottomScores?selected:panel);
    Text(GFX_BOTTOM,16,17,bottomScores?"< LOG / SCORES":"LOG / SCORES >",ink,17);
    Rect(GFX_BOTTOM,168,6,144,30,bottomSlow?selected:panel);
    Text(GFX_BOTTOM,180,17,bottomSlow?"WALK: HELD":"HOLD TO WALK",bottomSlow?accent:ink,15);
    Rect(GFX_BOTTOM,8,42,304,2,accent);
    if(!bottomScores) {
        unsigned rows=std::min(bottomCount,15u),first=(bottomLine+128+1-rows)%128;
        for(unsigned i=0;i<rows;++i) Text(GFX_BOTTOM,8,51+i*11,bottomLines[(first+i)%128],ink,38);
        Text(GFX_BOTTOM,8,223,"Drag below buttons to look",muted,38);
    } else {
        int order[MAXPLAYERS],count=0;
        for(int i=0;i<MAXPLAYERS;++i) if(playeringame[i]) order[count++]=i;
        std::stable_sort(order,order+count,[](int a,int b) {
            if(players[a].bSpectating!=players[b].bSpectating) return !players[a].bSpectating;
            return players[a].fragcount>players[b].fragcount;
        });
        unsigned pages=std::max(1,(count+12)/13); bottomPage%=pages;
        Text(GFX_BOTTOM,8,51,"PLAYER           FRAG  PTS  WIN",muted,38);
        for(int row=0;row<13 && int(bottomPage*13)+row<count;++row) {
            int index=order[bottomPage*13+row]; player_t &p=players[index];
            char stats[64];
            if(p.bSpectating) snprintf(stats,sizeof(stats),"%-16s  SPECTATOR",BottomName(p.userinfo.GetName()).c_str());
            else snprintf(stats,sizeof(stats),"%-16s %4d %4ld %4lu",BottomName(p.userinfo.GetName()).c_str(),p.fragcount,(long)p.lPointCount,(unsigned long)p.ulWins);
            Text(GFX_BOTTOM,8,67+row*11,stats,index==consoleplayer?accent:ink,38);
        }
        char footer[64]; snprintf(footer,sizeof(footer),"%d players  page %u/%u: tap next",count,bottomPage+1,pages);
        Text(GFX_BOTTOM,8,223,footer,muted,38);
    }
}

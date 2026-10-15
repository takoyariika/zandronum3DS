#include <3ds.h>
#include <cstdio>
#include <cstdlib>
#include <clocale>
#include <new>
#include <sys/stat.h>
#include <malloc.h>
#include <dirent.h>
#include <vector>
#include <string>
#include <algorithm>
#include "doomerrors.h"
#include "m_argv.h"
#include "d_main.h"
#include "i_system.h"
#include "c_console.h"
#include "cmdlib.h"
#include "version.h"
#include "doomstat.h"
#include "g_level.h"
#include "g_game.h"
#include "d_event.h"
#include "i_3ds.h"
#include "c_dispatch.h"
#include "csnd_3ds.h"
#include "doomlist_3ds.h"
#include "doomlist_http_3ds.h"
#include "launcher_ui_3ds.h"
void I3DS_SoundQuiesce();
void I3DS_SoundRelease();
void I3DS_SoundRestore();
#include "c_cvars.h"
EXTERN_CVAR(Bool, use_joystick)
void I_StartupJoysticks();
void I_ShutdownJoysticks();

extern "C" {
    u32 __stacksize__ = 1024 * 1024;
    // Software presentation needs ~1 MiB of linear LCD buffers, not 32 MiB.
    u32 __ctru_linear_heap_size = 4 * 1024 * 1024;
}
DArgs *Args;
static void (*terms[128])();
static unsigned numTerms;
static PrintConsole loadingConsole, bottomConsole;
static void ThemeConsole(PrintConsole &console) {
    console.fg=LauncherUI::ink; console.bg=LauncherUI::background;
    console.flags=CONSOLE_FG_CUSTOM|CONSOLE_BG_CUSTOM;
    consoleSelect(&console); consoleClear();
}
static bool loadingDisplay = true;
static bool launcherRequested;
static unsigned launcherReturns;
bool I3DS_RequestLauncherReturn() {
    bool diagnostic=false;
    for(int i=1;i<Args->NumArgs();++i) if(!strncmp(Args->GetArg(i),"-3ds-test",9)) diagnostic=true;
    if(diagnostic && !Args->CheckParm("-3ds-test-return-launcher")) return false;
    launcherRequested=true;
    I3DS_ReportExit("Game quit: return to launcher");
    C_DoCommand("restart"); // Unwinds into the engine's complete restart cleanup.
    I_FatalError("Engine restart did not return to launcher");
    return true;
}
static bool platformActive=true;
static u64 suspendedAt;
static unsigned resumeGeneration;
static aptHookCookie lifecycleHook;
static unsigned lifecycleCounts[5];
static int lastLifecycle=-1;
static u64 launchRtc;
static int observedState=-1;
static void WriteLifecycle(FILE *f,const char *reason)
{
    fprintf(f,"%s\nPort0.1.18 launch RTC(ms1900)=%llu current=%llu\nstate=%d action=%d map=%.255s heap=%u KiB active=%d close=%d home=%d\nlast hook=%d suspend=%u restore=%u sleep=%u wake=%u exit=%u\n",reason,(unsigned long long)launchRtc,(unsigned long long)osGetTime(),(int)gamestate,(int)gameaction,level.mapname,(unsigned)mallinfo().uordblks/1024,aptIsActive(),aptShouldClose(),aptShouldJumpToHome(),lastLifecycle,lifecycleCounts[APTHOOK_ONSUSPEND],lifecycleCounts[APTHOOK_ONRESTORE],lifecycleCounts[APTHOOK_ONSLEEP],lifecycleCounts[APTHOOK_ONWAKEUP],lifecycleCounts[APTHOOK_ONEXIT]);
}
void I3DS_ReportExit(const char *reason)
{
    FILE *f=fopen("sdmc:/3ds/zandronum/quit.txt","w");
    if(f) { WriteLifecycle(f,reason); fclose(f); }
}
bool I3DS_ConsoleActive() { return platformActive; }
unsigned I3DS_ResumeGeneration() { return resumeGeneration; }
static void Lifecycle(APT_HookType event, void *)
{
    lastLifecycle=(int)event;
    if((unsigned)event<5) ++lifecycleCounts[event];
    if (event==APTHOOK_ONSLEEP || event==APTHOOK_ONSUSPEND) {
        sndSetSuspended(true);
        I3DS_SoundQuiesce();
        if (event==APTHOOK_ONSUSPEND) I3DS_SoundRelease();
        if (platformActive) { suspendedAt=osGetTime(); platformActive=false; }
    } else if (event==APTHOOK_ONWAKEUP || event==APTHOOK_ONRESTORE) {
        if (event==APTHOOK_ONRESTORE) I3DS_SoundRestore();
        sndSetSuspended(false);
        if (!platformActive) { I3DS_AdjustTimer(osGetTime()-suspendedAt); ++resumeGeneration; platformActive=true; }
    } else if (event==APTHOOK_ONEXIT) platformActive=false;
}
bool I3DS_PumpEvents()
{
    // APT hooks may print/redraw through the engine. Let the outer pump finish
    // its suspend/restore handshake before processing another APT transition.
    static bool pumping;
    if(pumping) return true;
    pumping=true;
    struct PumpGuard { bool &busy; ~PumpGuard() { busy=false; } } guard={pumping};
    // Only scene transitions, never per-frame/periodic filesystem logging.
    if(!loadingDisplay && observedState!=(int)gamestate) {
        observedState=(int)gamestate;
        FILE *f=fopen("sdmc:/3ds/zandronum/transition.txt","a");
        if(f) { WriteLifecycle(f,"Entering scene before presentation"); fclose(f); }
    }
    // Background/sleep is a pause, not a request to terminate the title.
    while (aptMainLoop()) {
        if (aptIsActive()) {
            if (!platformActive) Lifecycle(APTHOOK_ONRESTORE, NULL);
            return true;
        }
        svcSleepThread(1000000);
    }
    FILE *f=fopen("sdmc:/3ds/zandronum/apt-exit.txt","w");
    if(f) { WriteLifecycle(f,"APT requested application close"); fclose(f); }
    return false;
}
void I3DS_TestSleep()
{
    unsigned before=I_MSTime();
    Lifecycle(APTHOOK_ONSLEEP,NULL);
    svcSleepThread(50000000);
    Lifecycle(APTHOOK_ONWAKEUP,NULL);
    Printf("3DS SLEEP TEST: clock delta=%u ms, active=%d, generation=%u\n",I_MSTime()-before,platformActive,resumeGeneration);
}
void I3DS_LoadingText(const char *text)
{
    if (!loadingDisplay) return;
    if (!I3DS_PumpEvents()) exit(0);
    consoleSelect(&loadingConsole);
    fputs(text, stdout);
    consoleSelect(&bottomConsole);
}
void I3DS_EndLoadingDisplay()
{
    if (!loadingDisplay) return;
    loadingDisplay = false;
    gfxSetScreenFormat(GFX_TOP, GSP_BGR8_OES);
    gfxSetDoubleBuffering(GFX_TOP, true);
}
bool I3DS_GameDisplayActive() { return !loadingDisplay; }
void addterm(void (*func)(), const char *)
{
    for (unsigned i=0; i<numTerms; ++i) if (terms[i] == func) return;
    if (numTerms == 128) I_FatalError("Too many exit handlers");
    terms[numTerms++] = func;
}
void popterm() { if (numTerms) --numTerms; }
void call_terms() { while (numTerms) terms[--numTerms](); }
void I3DS_WaitFrame() { svcSleepThread(1000000000LL/60); }
void I3DS_FatalScreen(const char *text)
{
    FILE *f = fopen("sdmc:/3ds/zandronum/fatal.txt", "w");
    if (f) { fputs(text, f); fclose(f); }
    // The game's last LCD buffers otherwise remain visible on physical hardware.
    // Fatal mode must present console writes explicitly, not just print to stdout.
    sndSetSuspended(true);
    I3DS_SoundQuiesce();
    sndEmergencyStop();
    consoleInit(GFX_TOP,&loadingConsole);
    ThemeConsole(loadingConsole); ThemeConsole(bottomConsole);
    consoleSelect(&loadingConsole); consoleClear();
    printf("FATAL ERROR\n\n%s\n\nSaved: 3ds/zandronum/fatal.txt\nSTART: HOME\n",text);
    consoleSelect(&bottomConsole); consoleClear();
    printf("FATAL ERROR\n\n%s\n\nHeap %u/%u KiB\nSTART: HOME\n",text,(unsigned)mallinfo().uordblks/1024,(unsigned)envGetHeapSize()/1024);
    gfxFlushBuffers(); gfxSwapBuffers(); gspWaitForVBlank();
    unsigned fatalStarted=I_MSTime();
    while (aptMainLoop())
    {
        gfxFlushBuffers(); gfxSwapBuffers(); gspWaitForVBlank();
        hidScanInput();
        if(Args && Args->CheckParm("-3ds-test-fatal") && I_MSTime()-fatalStarted>=1000) {
            u16 w,h; u8 *pixels=gfxGetFramebuffer(GFX_BOTTOM,GFX_LEFT,&w,&h);
            FILE *dump=fopen("sdmc:/3ds/zandronum/fatal-bottom.rgb565","wb");
            if(dump) { u16 size[]={w,h}; fwrite(size,sizeof(size),1,dump); fwrite(pixels,2,(size_t)w*h,dump); fclose(dump); }
            break;
        }
        if (hidKeysDown() & KEY_START) break;
        I3DS_WaitFrame();
    }
}
static void *socBuffer;
static Result networkInitResult;
static bool diagnostics, networkEnabled;
bool I3DS_Diagnostics() { return diagnostics; }
bool I3DS_NetworkEnabled() { return networkEnabled; }
static void StopNetwork()
{
    networkEnabled=false;
    if(socBuffer) { socExit(); free(socBuffer); socBuffer=NULL; }
}
static bool StartNetwork()
{
    if(networkEnabled) return true;
    socBuffer=memalign(0x1000,1024*1024);
    if(!socBuffer) { networkInitResult=(Result)0xffffffff; return false; }
    Result rc=socInit((u32*)socBuffer,1024*1024);
    networkInitResult=rc;
    if(R_FAILED(rc)) { free(socBuffer); socBuffer=NULL; return false; }
    networkEnabled=true; return true;
}
static void ShutdownPlatform()
{
    platformActive=false;
    aptUnhook(&lifecycleHook);
    StopNetwork();
    gfxExit();
}
struct LaunchFile { std::string path; bool iwad; };
static std::string FileName(std::string path)
{
    size_t slash=path.find_last_of("/\\");
    if(slash!=std::string::npos) path.erase(0,slash+1);
    for(char &c:path) c=tolower((unsigned char)c);
    return path;
}
static bool ServerFiles(const DoomServer &server,const std::vector<LaunchFile> &files,
                        std::string &iwad,std::vector<std::string> &mods,std::string &missing)
{
    iwad.clear(); mods.clear(); missing.clear();
    auto find=[&](const std::string &name,bool iw) {
        // The API supplies filenames, never SD paths. Match case-insensitively.
        if(name.empty() || name.find_first_of("/\\:")!=std::string::npos) return std::string();
        for(auto &f:files) if(f.iwad==iw && FileName(f.path)==FileName(name)) return f.path;
        return std::string();
    };
    iwad=find(server.iwad,true);
    if(iwad.empty()) missing+=server.iwad+"\n";
    for(auto &w:server.wads) {
        if(FileName(w)=="zandronum.pk3" || FileName(w)=="skulltag_actors.pk3") continue;
        std::string path=find(w,false);
        if(path.empty()) missing+=w+"\n"; else mods.push_back(path);
    }
    return missing.empty();
}
static std::string ScreenText(const std::string &s)
{
    std::string out;
    for(size_t n=0;n<s.size();++n) {
        unsigned char c=s[n];
        if(c==0x1c && n+1<s.size()) { ++n; continue; }
        // Never interpret a server name's terminal escape/control characters.
        if(c>=32 && c<127) out+=char(c); else if(c>=128) out+='?';
    } return out;
}
static bool FetchServers(std::vector<DoomServer> &servers,std::string &error)
{
    servers.clear(); error.clear();
    LauncherUI::Message("SERVER BROWSER","Fetching doomlist.net... B cancels.");
    extern bool I3DS_FetchDoomList(std::string &,std::string &);
    std::string json;
    if(!I3DS_FetchDoomList(json,error)) return false;
    if(!DoomListReader(json).read(servers)) { error="Invalid server list response"; return false; }
    servers.erase(std::remove_if(servers.begin(),servers.end(),[](const DoomServer &s) { return !DoomServerCompatible(s); }),servers.end());
    std::sort(servers.begin(),servers.end(),[](const DoomServer &a,const DoomServer &b) {
        return a.players!=b.players ? a.players>b.players : a.name<b.name;
    });
    return true;
}
static bool AskPassword(const char *hint,std::string &value)
{
    SwkbdState keyboard; swkbdInit(&keyboard,SWKBD_TYPE_NORMAL,2,127);
    swkbdSetHintText(&keyboard,hint); swkbdSetPasswordMode(&keyboard,SWKBD_PASSWORD_HIDE);
    char text[128]={};
    if(swkbdInputText(&keyboard,text,sizeof(text))!=SWKBD_BUTTON_CONFIRM) return false;
    value=text; memset(text,0,sizeof(text)); return true;
}
static bool AskText(const char *hint,std::string &value)
{
    SwkbdState keyboard; swkbdInit(&keyboard,SWKBD_TYPE_NORMAL,2,253);
    swkbdSetHintText(&keyboard,hint); swkbdSetInitialText(&keyboard,value.c_str());
    char text[254]={};
    if(swkbdInputText(&keyboard,text,sizeof(text))!=SWKBD_BUTTON_CONFIRM) return false;
    value=text; return true;
}
static void ScanLaunchFiles(const std::string &,int,std::vector<LaunchFile> &);
static std::string downloadName;
static u64 downloadStarted;
static void DownloadProgress(size_t bytes,size_t total)
{
    using namespace LauncherUI;
    Begin("DOWNLOADING WAD","SD CARD / B: CANCEL");
    Wrapped(GFX_TOP,16,58,downloadName,46,3);
    char progress[80]; snprintf(progress,sizeof(progress),"%u%%   %u / %u KiB",(unsigned)(bytes*100/total),(unsigned)(bytes/1024),(unsigned)(total/1024));
    Text(GFX_TOP,16,112,progress,accent); Rect(GFX_TOP,16,135,368,8,panel);
    Rect(GFX_TOP,16,135,368*bytes/total,8,accent);
    Text(GFX_BOTTOM,16,30,"Saving to SD card",accent);
    Wrapped(GFX_BOTTOM,16,55,downloadName,36,4,muted);
    extern unsigned I3DS_DownloadConnections();
    u64 elapsed=osGetTime()-downloadStarted;
    snprintf(progress,sizeof(progress),"%u KiB/s   %u connection(s)",elapsed?(unsigned)((u64)bytes*1000/elapsed/1024):0,I3DS_DownloadConnections());
    Text(GFX_BOTTOM,16,118,progress,accent);
    Text(GFX_BOTTOM,16,142,"Completed files stay in downloads/",muted,36);
    Button(0,1,"B Cancel"); Present();
}
static bool DownloadMissing(const DoomServer &server,std::vector<LaunchFile> &files,std::string &error)
{
    extern bool I3DS_DownloadWad(const std::string &,const std::string &,std::string &,void (*)(size_t,size_t));
    const std::string root="sdmc:/3ds/zandronum";
    mkdir((root+"/downloads").c_str(),0777);
    for(auto &name:server.wads) {
        std::string lower=FileName(name);
        if(lower=="zandronum.pk3" || lower=="skulltag_actors.pk3") continue;
        bool found=false; for(auto &file:files) if(!file.iwad && FileName(file.path)==lower) found=true;
        if(found) continue;
        if(!DoomDownloadFilename(name)) { error="Invalid WAD filename: "+name; return false; }
        downloadName=name; downloadStarted=osGetTime(); LauncherUI::Message("FINDING WAD",name);
        if(!I3DS_DownloadWad(lower,root+"/downloads/"+lower,error,DownloadProgress)) { error=name+": "+error; return false; }
        files.clear(); ScanLaunchFiles(root,0,files);
    }
    error.clear(); return true;
}
static bool ServerDetails(const DoomServer &server,std::vector<LaunchFile> &files)
{
    using namespace LauncherUI;
    unsigned offset=0; std::string error; bool redraw=true;
    for(;;) {
        std::string iwad,missing; std::vector<std::string> mods;
        bool ready=ServerFiles(server,files,iwad,mods,missing);
        std::vector<std::string> names={server.iwad}; names.insert(names.end(),server.wads.begin(),server.wads.end());
        if(redraw) {
        Begin("SERVER FILES","ORDERED WADS / L-R: PAGE");
        for(unsigned row=0;row<6 && offset+row<names.size();++row) {
            unsigned n=offset+row; bool found=false;
            for(auto &file:files) if(FileName(file.path)==FileName(names[n])) found=true;
            if(FileName(names[n])=="zandronum.pk3" || FileName(names[n])=="skulltag_actors.pk3") found=true;
            Card(row,names[n],n==0?"BASE GAME / SUPPLY YOUR IWAD":"MOD / LOAD ORDER "+std::to_string(n),false,found?"SAVED":"MISSING");
        }
        Text(GFX_TOP,16,221,std::to_string(names.size())+" files   "+server.address,muted);
        Wrapped(GFX_BOTTOM,12,17,ScreenText(server.name),37,2);
        Text(GFX_BOTTOM,12,49,ready?"READY TO CONNECT":"REQUIRED FILES MISSING",ready?accent:warning);
        Wrapped(GFX_BOTTOM,12,70,error.empty()?(ready?"START joins this server.":"X downloads missing MOD files. Base game IWAD must be on SD."):error,37,7,error.empty()?muted:warning);
        Button(0,0,"X Download",selected); Button(1,0,"START Connect",selected);
        Button(0,1,"L / R Page"); Button(1,1,"B Back"); Present();
        redraw=false;
        }
        if(!I3DS_PumpEvents()) return false;
        hidScanInput(); u32 key=hidKeysDown(); key|=TouchButtons(key,KEY_X,KEY_START,KEY_R,KEY_B);
        if(key & KEY_B) return false;
        if(key & KEY_START) { if(ready) return true; error="Download the missing MOD files first."; }
        if(key & KEY_X) { DownloadMissing(server,files,error); }
        if(key & KEY_L) offset=offset>=6?offset-6:0;
        if(key & KEY_R) { if(offset+6<names.size()) offset+=6; else offset=0; }
        if(key) redraw=true;
        I3DS_WaitFrame();
    }
}
static void TestDoomList()
{
    auto check=[](bool ok) { if(!ok) I_FatalError("3DS doomlist regression failed"); };
    const std::string fixture=R"({"127.0.0.1:10666":{"name":"Test\u0020server","iwad":"doom2.wad","numplayers":2,"maxclients":8,"forcepassword":true,"pwads":["first.wad","Second.pk3"],"unused":{"nested":[1,true,null,{"x":"y"}]}}})";
    std::vector<DoomServer> servers;
    check(DoomListReader(fixture).read(servers));
    check(servers.size()==1 && servers[0].name=="Test server" && servers[0].players==2 && servers[0].password);
    servers[0].version="3.2.1-r250804-2140"; check(DoomServerCompatible(servers[0]));
    servers[0].version="3.3-alpha-r260925-2352"; check(!DoomServerCompatible(servers[0]));
    servers[0].version="3.2.1-alpha-r250501-0000"; check(!DoomServerCompatible(servers[0]));
    servers[0].version="3.2.10"; check(!DoomServerCompatible(servers[0]));
    servers[0].version="3.2.1-tspg32"; check(DoomServerCompatible(servers[0]));
    check(DoomServerMatches(servers[0],"TEST") && DoomServerMatches(servers[0],"SECOND.PK3") && !DoomServerMatches(servers[0],"absent"));
    std::string direct;
    check(DoomDirectAddress(" 127.0.0.1:10666 ",direct) && direct=="127.0.0.1:10666");
    check(DoomDirectAddress("Example.COM",direct) && direct=="example.com:10666");
    check(!DoomDirectAddress("127.0.0.1:65536",direct) && !DoomDirectAddress("host:0",direct) && !DoomDirectAddress("host;quit",direct));
    check(DoomDownloadFilename("tspg_connectioncolumn_r1.wad") && DoomDownloadFilename("skulltag_content-3.2-beta2.pk3"));
    check(!DoomDownloadFilename("../bad.wad") && !DoomDownloadFilename("bad.exe") && !DoomDownloadFilename(std::string("bad\0.wad",8)));
    std::vector<LaunchFile> files={{"sdmc:/DOOM2.WAD",true},{"sdmc:/mods/FIRST.WAD",false},{"sdmc:/Second.pk3",false}};
    std::string iwad,missing; std::vector<std::string> mods;
    check(ServerFiles(servers[0],files,iwad,mods,missing));
    check(iwad==files[0].path && mods.size()==2 && mods[0]==files[1].path && mods[1]==files[2].path);
    servers[0].wads.push_back("../FIRST.WAD");
    check(!ServerFiles(servers[0],files,iwad,mods,missing));
    check(missing=="../FIRST.WAD\n");
    check(!DoomListReader(fixture.substr(0,fixture.size()-1)).read(servers));
    check(!DoomListReader(fixture+"garbage").read(servers));
    check(DoomListReader("{}").read(servers) && servers.empty());
    std::string deep(40,'['); deep+="0"; deep+=std::string(40,']');
    check(!DoomListReader("{\"127.0.0.1:1\":{\"unused\":"+deep+"}}").read(servers));
    check(ScreenText("\x1b[2Jhello\n")=="[2Jhello");
    check(ScreenText(std::string("\x1c")+"gTitle")=="Title");
    std::string body;
    check(DoomListHttpBody("HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\n{}",body) && body=="{}");
    check(DoomListHttpBody("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n1\r\n{\r\n1\r\n}\r\n0\r\n\r\n",body) && body=="{}");
    check(!DoomListHttpBody("HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\n{}",body));
    check(!DoomListHttpBody("HTTP/1.1 503 Error\r\n\r\n{}",body));
    FILE *api=fopen("sdmc:/3ds/zandronum/doomlist-test.json","rb");
    if(api) {
        std::string json; char buffer[4096]; size_t n;
        while((n=fread(buffer,1,sizeof(buffer),api)) && json.size()<2*1024*1024) json.append(buffer,n);
        fclose(api); check(DoomListReader(json).read(servers));
        check(!servers.empty());
        printf("3DS DOOMLIST TEST: public API parsed %u servers\n",(unsigned)servers.size());
    }
    FILE *result=fopen("sdmc:/3ds/zandronum/doomlist-test.txt","w");
    if(result) { fprintf(result,"PASS: JSON and ordered local WAD validation; public servers=%u\n",(unsigned)servers.size()); fclose(result); }
    printf("3DS DOOMLIST TEST: parsing, ordered WAD matching, missing files, malformed JSON and controls OK\n");
}
static std::string connectionPassword, joinPassword;
#include "server_browser_3ds.inc"
static void ScanLaunchFiles(const std::string &root, int depth, std::vector<LaunchFile> &files)
{
    DIR *dir=opendir(root.c_str()); if (!dir) return;
    while (dirent *entry=readdir(dir)) {
        if (entry->d_name[0]=='.') continue;
        std::string path=root+"/"+entry->d_name; struct stat st;
        if (stat(path.c_str(), &st)) continue;
        if (S_ISDIR(st.st_mode)) { if (depth<3) ScanLaunchFiles(path,depth+1,files); continue; }
        std::string name=entry->d_name;
        for (char &c:name) c=tolower((unsigned char)c);
        if (name=="zandronum.pk3" || name=="skulltag_actors.pk3") continue;
        size_t dot=name.rfind('.'); if (dot==std::string::npos) continue;
        std::string ext=name.substr(dot);
        if (ext!=".wad" && ext!=".pk3" && ext!=".zip") continue;
        FILE *f=fopen(path.c_str(),"rb"); char magic[4]={};
        if (f) { fread(magic,1,4,f); fclose(f); }
        bool iwad=!memcmp(magic,"IWAD",4);
        if (ext==".wad" && !iwad && memcmp(magic,"PWAD",4)) continue;
        files.push_back({path,iwad});
    }
    closedir(dir);
}
static bool ChooseGame()
{
    const std::string root="sdmc:/3ds/zandronum";
    std::vector<LaunchFile> files; ScanLaunchFiles(root,0,files);
    std::sort(files.begin(),files.end(),[](const LaunchFile&a,const LaunchFile&b) {
        return a.iwad!=b.iwad ? a.iwad>b.iwad : a.path<b.path;
    });
    std::string iwad; std::vector<std::string> mods;
    FILE *saved=fopen((root+"/launch.txt").c_str(),"r");
    if (saved) {
        char line[1024]; bool first=true;
        while (fgets(line,sizeof(line),saved)) {
            line[strcspn(line,"\r\n")]=0;
            if (first) { iwad=line; first=false; } else if (*line) mods.push_back(line);
        } fclose(saved);
    } else {
        if (const char *p=Args->CheckValue("-iwad")) iwad=p;
        int f=Args->CheckParm("-file");
        for (int i=f+1; f && i<Args->NumArgs(); ++i) {
            const char *p=Args->GetArg(i); if (*p=='-' || *p=='+') break; mods.push_back(p);
        }
    }
    auto exists=[&files](const std::string&p,bool iw) { for(auto &f:files) if(f.path==p && f.iwad==iw) return true; return false; };
    if (!exists(iwad,true)) { iwad.clear(); for(auto &f:files) if(f.iwad) { iwad=f.path; break; } }
    mods.erase(std::remove_if(mods.begin(),mods.end(),[&](const std::string&p){return !exists(p,false);}),mods.end());
    if(Args->CheckParm("-3ds-test-return-launcher")) mods.clear();
    unsigned cursor=0; bool redraw=true;
    unsigned testStep=0;
    for (;;) {
        if (redraw) {
            using namespace LauncherUI;
            Begin("ZANDRONUM", "3.2.1 / NEW 3DS / SELECT GAME FILES");
            unsigned begin=(cursor/6)*6;
            for(unsigned row=0;row<6 && begin+row<files.size();++row) {
                unsigned n=begin+row;
                auto m=std::find(mods.begin(),mods.end(),files[n].path);
                std::string badge=files[n].iwad?(files[n].path==iwad?"ACTIVE":"BASE"):(m==mods.end()?"":"MOD "+std::to_string(m-mods.begin()+1));
                Card(row,files[n].path.substr(root.size()+1),files[n].iwad?"BASE GAME":"MOD / A TO TOGGLE",n==cursor,badge);
            }
            Text(GFX_TOP,16,221,"D-pad: select   L/R: page   A: toggle",muted);
            Text(GFX_BOTTOM,12,20,"YOUR GAME",accent);
            Wrapped(GFX_BOTTOM,12,45,iwad.empty()?"Select a base game IWAD.":FileName(iwad),37,2);
            Text(GFX_BOTTOM,12,78,std::to_string(mods.size())+" MOD files selected",muted);
            if(!files.empty()) Wrapped(GFX_BOTTOM,12,105,files[cursor].path.substr(root.size()+1),37,3,ink);
            if(files.empty()) Wrapped(GFX_BOTTOM,12,105,"Copy game files to SD:/3ds/zandronum/",37,3,warning);
            Button(0,0,"START Play",selected); Button(1,0,"Y Network",selected);
            Button(0,1,"X Clear MODs"); Button(1,1,"B Exit to HOME"); Present();
            redraw=false;
        }
        if(!I3DS_PumpEvents()) return false;
        hidScanInput(); u32 key=hidKeysDown(); key|=LauncherUI::TouchButtons(key,KEY_START,KEY_Y,KEY_X,KEY_B);
        if (Args->CheckParm("-3ds-test-launcher")) {
            if(testStep==0) key=KEY_X;
            else if(testStep==1) {
                for(unsigned n=0;n<files.size();++n) if(files[n].path==root+"/BYOCv2.0.pk3") cursor=n;
                key=KEY_A;
            } else if(testStep==2) {
                bool found=false;
                for(unsigned n=0;n<files.size();++n) if(files[n].path==root+"/Chooser test.wad") { cursor=n; found=true; }
                key=found?KEY_A:KEY_START;
            } else key=KEY_START;
            ++testStep;
        }
        if(Args->CheckParm("-3ds-test-launcher-generic")) { mods.clear(); key=KEY_START; }
        if(Args->CheckParm("-3ds-test-return-launcher") && launcherReturns) {
            if(testStep++==0) {
                LauncherUI::Dump("ui-return-launcher");
                FILE *report=fopen((root+"/return-launcher.txt").c_str(),"a");
                if(report) { fprintf(report,"launcher %u: network=%d heap=%u KiB\n",launcherReturns,networkEnabled,(unsigned)mallinfo().uordblks/1024); fclose(report); }
            }
            key=launcherReturns>=2?KEY_B:KEY_START;
        }
        if(key & KEY_B) {
            I3DS_ReportExit("Launcher: exit to HOME");
            if(Args->CheckParm("-3ds-test-return-launcher")) {
                FILE *report=fopen((root+"/return-launcher.txt").c_str(),"a");
                if(report) { fputs("PASS: two game quits returned to launcher, relaunch played, B exits to HOME\n",report); fclose(report); }
            }
            return false;
        }
        if(key & KEY_Y) {
            if(ChooseServer(files,iwad,mods)) break;
            if(!I3DS_PumpEvents()) return false;
            redraw=true;
        }
        if(key & KEY_X) { mods.clear(); redraw=true; }
        if(!files.empty()) {
            if(key & KEY_DUP) { cursor=(cursor+files.size()-1)%files.size(); redraw=true; }
            if(key & KEY_DDOWN) { cursor=(cursor+1)%files.size(); redraw=true; }
            if(key & KEY_L) { cursor=cursor>=6?cursor-6:0; redraw=true; }
            if(key & KEY_R) { cursor=std::min(cursor+6,(unsigned)files.size()-1); redraw=true; }
            if(key & KEY_A) {
                auto &f=files[cursor]; if(f.iwad) iwad=f.path;
                else { auto m=std::find(mods.begin(),mods.end(),f.path); if(m==mods.end()) mods.push_back(f.path); else mods.erase(m); }
                redraw=true;
            }
        }
        if((key & KEY_START) && !iwad.empty()) break;
        I3DS_WaitFrame();
    }
    saved=(Args->CheckParm("-3ds-test-launcher") || Args->CheckParm("-3ds-test-return-launcher")) ? NULL : fopen((root+"/launch.txt").c_str(),"w");
    if(saved) { fprintf(saved,"%s\n",iwad.c_str()); for(auto &m:mods) fprintf(saved,"%s\n",m.c_str()); fclose(saved); }
    for (const char *option : {"-iwad","-file"}) {
        int pos;
        while ((pos=Args->CheckParm(option))) {
            Args->RemoveArg(pos);
            while(pos<Args->NumArgs() && Args->GetArg(pos)[0]!='-' && Args->GetArg(pos)[0]!='+') Args->RemoveArg(pos);
        }
    }
    Args->AppendArg("-iwad"); Args->AppendArg(iwad.c_str());
    if(!mods.empty()) { Args->AppendArg("-file"); for(auto&m:mods) Args->AppendArg(m.c_str()); }
    if(Args->CheckParm("-3ds-test-launcher-generic")) {
        if(!mods.empty() || Args->CheckParm("-file")) I_FatalError("Generic launcher unexpectedly selected a MOD");
        struct stat absent;
        FILE *report=fopen((root+"/launcher-test.txt").c_str(),"w");
        if(report) { fprintf(report,"PASS: IWAD=%s mods=0; optional fixture %s\n",iwad.c_str(),stat((root+"/BYOCv2.0.pk3").c_str(),&absent)?"absent":"present"); fclose(report); }
    }
    consoleSelect(&loadingConsole); consoleClear(); consoleSelect(&bottomConsole); consoleClear();
    printf("3DS LAUNCH: IWAD=%s, mods=%u\n",iwad.c_str(),(unsigned)mods.size());
    printf("\x1b[2J\x1b[1;1HLoading selected files...\n");
    return true;
}
void I3DS_RestartLauncher() {
    if(!launcherRequested) return;
    launcherRequested=false; ++launcherReturns;
    StopNetwork();
    I3DS_ResetBottomScreen();
    connectionPassword.clear(); joinPassword.clear();
    extern FStringCVar cl_password,cl_joinpassword;
    cl_password=""; cl_joinpassword="";
    gamestate=GS_STARTUP; gameaction=ga_nothing;
    loadingDisplay=true;
    gfxSetDoubleBuffering(GFX_TOP,false);
    consoleInit(GFX_TOP,&loadingConsole); consoleInit(GFX_BOTTOM,&bottomConsole);
    ThemeConsole(loadingConsole); ThemeConsole(bottomConsole);
    // Do not retain a connection or startup map from the previous session.
    for(const char *option:{"+connect","-connect","-host","+map","-warp","+load","-loadgame"})
        while(Args->CheckParm(option)) Args->RemoveArgs(option);
    if(!ChooseGame()) exit(0);
    if(networkEnabled) { cl_password=connectionPassword.c_str(); cl_joinpassword=joinPassword.c_str(); }
    connectionPassword.clear(); joinPassword.clear();
}
int main(int argc, char **argv)
{
    osSetSpeedupEnable(true);
    gfxInitDefault();
    consoleInit(GFX_TOP, &loadingConsole);
    consoleInit(GFX_BOTTOM, &bottomConsole);
    ThemeConsole(loadingConsole); ThemeConsole(bottomConsole);
    aptHook(&lifecycleHook, Lifecycle, NULL);
    mkdir("sdmc:/3ds", 0777);
    mkdir("sdmc:/3ds/zandronum", 0777);
    mkdir("sdmc:/3ds/zandronum/save", 0777);
    mkdir("sdmc:/3ds/zandronum/screenshots", 0777);
    // Do not let an old failure be confused with this launch.
    remove("sdmc:/3ds/zandronum/fatal.txt");
    remove("sdmc:/3ds/zandronum/performance.txt");
    remove("sdmc:/3ds/zandronum/apt-exit.txt");
    remove("sdmc:/3ds/zandronum/transition.txt");
    remove("sdmc:/3ds/zandronum/quit.txt");
    remove("sdmc:/3ds/zandronum/return-launcher.txt");
    launchRtc=osGetTime();
    I3DS_LoadingText("Zandronum 3DS\nLoading game data...\nLarge MODs may take some time.\n\n");
    FILE *videoLog=fopen("sdmc:/3ds/zandronum/video.txt","w");
    if(videoLog) { fputs("0.1.18 startup: awaiting first LCD presentation\n",videoLog); fclose(videoLog); }
    atexit(ShutdownPlatform);
    printf("Zandronum 3DS: %s\n", GetVersionString());
    printf("Heap: %lu MiB, linear: %lu MiB\n", (unsigned long)(envGetHeapSize()/1048576), (unsigned long)(envGetLinearHeapSize()/1048576));
    setlocale(LC_ALL, "C");
    std::set_new_handler([] {
        extern unsigned Z3DS_PurgeResourceCache();
        if (!Z3DS_PurgeResourceCache()) {
            struct mallinfo info=mallinfo();
            char detail[192];
            snprintf(detail,sizeof(detail),"Out of memory (resource cache already purged)\nHeap used: %u KiB / %u KiB; allocator free: %u KiB",(unsigned)info.uordblks/1024,(unsigned)envGetHeapSize()/1024,(unsigned)info.fordblks/1024);
            I_FatalError("%s",detail);
        }
    });
    atexit(call_terms);
    try
    {
        static char appName[] = "zandronum3ds";
        char *initialArgs[] = {appName, NULL};
        Args = new DArgs(1, initialArgs);
        for (int i=1; i<argc; ++i) Args->AppendArg(argv[i]);
        Args->AppendArg("-config"); Args->AppendArg("sdmc:/3ds/zandronum/zandronum.ini");
        Args->AppendArg("+vid_defwidth"); Args->AppendArg("400");
        Args->AppendArg("+vid_defheight"); Args->AppendArg("240");
        Args->AppendArg("+logfile"); Args->AppendArg("sdmc:/3ds/zandronum/engine.log");
        FILE *af = fopen("sdmc:/3ds/zandronum/args.txt", "r");
        if (af)
        {
            char word[512];
            while (fscanf(af, "%511s", word) == 1) Args->AppendArg(word);
            fclose(af);
        }
        bool test=Args->CheckParm("-3ds-test") || Args->CheckParm("-3ds-test-input") || Args->CheckParm("-3ds-test-quit") || Args->CheckParm("-3ds-test-sleep");
        diagnostics=Args->CheckParm("-3ds-diagnostics");
        for(int i=1;i<Args->NumArgs();++i) if(!strncmp(Args->GetArg(i),"-3ds-test",9)) diagnostics=true;
        if(Args->CheckParm("-3ds-quiet")) diagnostics=false;
        // A saved config/args connection must not silently turn offline on.
        for(const char *option:{"-connect","+connect","-host"}) {
            int pos; while((pos=Args->CheckParm(option))) {
                Args->RemoveArg(pos);
                while(pos<Args->NumArgs() && Args->GetArg(pos)[0]!='-' && Args->GetArg(pos)[0]!='+') Args->RemoveArg(pos);
            }
        }
        if ((!test || Args->CheckParm("-3ds-test-launcher") || Args->CheckParm("-3ds-test-launcher-generic")) && !ChooseGame()) return 0;
        if(Args->CheckParm("-3ds-test-doomlist")) TestDoomList();
        if(Args->CheckParm("-3ds-test-browser")) {
            std::vector<LaunchFile> files; ScanLaunchFiles("sdmc:/3ds/zandronum",0,files);
            std::string iwad; std::vector<std::string> mods;
            ChooseServer(files,iwad,mods);
        }
        if(Args->CheckParm("-3ds-test-download")) {
            if(!StartNetwork()) I_FatalError("Download test network initialization failed");
            mkdir("sdmc:/3ds/zandronum/download-test",0777);
            FILE *report=fopen("sdmc:/3ds/zandronum/download-test.txt","w");
            for(const char *name:{"tspg_connectioncolumn_r1.wad","skulltag_content-3.2-beta2.pk3"}) {
                if(Args->CheckParm("-3ds-test-download-large") && strcmp(name,"skulltag_content-3.2-beta2.pk3")) continue;
                std::string error,path=std::string("sdmc:/3ds/zandronum/download-test/")+name;
                if(!Args->CheckParm("-3ds-test-download-reuse")) remove(path.c_str()); // Only diagnostic fixtures.
                struct stat before={}; bool existed=stat(path.c_str(),&before)==0;
                downloadName=name; downloadStarted=osGetTime();
                extern bool I3DS_DownloadWad(const std::string &,const std::string &,std::string &,void (*)(size_t,size_t));
                bool ok=I3DS_DownloadWad(name,path,error,DownloadProgress);
                if(ok && existed) { struct stat after={}; ok=!stat(path.c_str(),&after) && before.st_size==after.st_size && before.st_mtime==after.st_mtime; }
                if(ok) { std::vector<LaunchFile> local; ScanLaunchFiles("sdmc:/3ds/zandronum",0,local); bool found=false; for(auto &f:local) if(f.path==path) found=true; ok=found; }
                if(report) { fprintf(report,"%s: %s %s\n",ok?"PASS":"FAIL",name,error.c_str()); fflush(report); }
                if(!ok) { if(report) fclose(report); I_FatalError("WAD download test failed: %s",error.c_str()); }
            }
            if(report) fclose(report); StopNetwork();
        }
        if(Args->CheckParm("-3ds-test-http")) {
            if(!StartNetwork()) I_FatalError("Test network initialization failed");
            std::vector<DoomServer> servers; std::string error;
            bool ok=FetchServers(servers,error);
            FILE *result=fopen("sdmc:/3ds/zandronum/doomlist-http.txt","w");
            if(result) { fprintf(result,"%s: servers=%u %s\n",ok?"PASS":"FAIL",(unsigned)servers.size(),error.c_str()); fclose(result); }
            StopNetwork();
            if(!ok) I_FatalError("Doomlist HTTPS test failed: %s",error.c_str());
        }
        if(const char *address=Args->CheckValue("-3ds-test-network")) {
            if(!StartNetwork()) I_FatalError("Test network initialization failed");
            Args->AppendArg("-connect"); Args->AppendArg(address);
        }
        if (test && !Args->CheckParm("-iwad")) { Args->AppendArg("-iwad"); Args->AppendArg("sdmc:/3ds/zandronum/DOOM2.WAD"); }
        if(networkEnabled) {
            extern FStringCVar cl_password, cl_joinpassword;
            cl_password=connectionPassword.c_str(); cl_joinpassword=joinPassword.c_str();
            connectionPassword.clear(); joinPassword.clear();
        }
        progdir = "sdmc:/3ds/zandronum/";
        UCVarValue enabled; enabled.Bool = true;
        use_joystick.SetGenericRepDefault(enabled, CVAR_Bool);
        I_StartupJoysticks();
        C_InitConsole(640, 200, false);
        atterm(I_Quit);
        D_DoomMain();
    }
    catch (CDoomError &error) { I3DS_FatalScreen(error.GetMessage()); return 1; }
    return 0;
}

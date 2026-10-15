static bool ConnectSelectedServer(const DoomServer &server,std::vector<LaunchFile> &files,
                                  std::string &iwad,std::vector<std::string> &mods)
{
    std::string requiredIwad,missing; std::vector<std::string> requiredMods;
    if(!ServerFiles(server,files,requiredIwad,requiredMods,missing)) {
        if(!ServerDetails(server,files) || !ServerFiles(server,files,requiredIwad,requiredMods,missing)) return false;
    }
    connectionPassword.clear(); joinPassword.clear();
    if(server.password && !AskPassword("Server password",connectionPassword)) return false;
    if(server.joinpassword && !AskPassword("Join password",joinPassword)) return false;
    iwad=requiredIwad; mods=requiredMods;
    Args->AppendArg("-connect"); Args->AppendArg(server.address.c_str());
    return true;
}
static bool ChooseServer(std::vector<LaunchFile> &files,std::string &iwad,std::vector<std::string> &mods)
{
    using namespace LauncherUI;
    std::vector<DoomServer> servers; std::vector<unsigned> visible;
    std::string error,query; unsigned cursor=0; bool redraw=true;
    auto filter=[&]() {
        visible.clear(); for(unsigned n=0;n<servers.size();++n) if(DoomServerMatches(servers[n],query)) visible.push_back(n);
        cursor=0; redraw=true;
    };
    auto refresh=[&]() {
        Message("SERVER BROWSER","Connecting to doomlist.net... B cancels.");
        if(StartNetwork()) FetchServers(servers,error);
        else { char message[80]; snprintf(message,sizeof(message),"SOC init failed: %08lx",(unsigned long)networkInitResult); error=message; }
        filter();
    };
    refresh();
    unsigned browserTestStep=0;
    for(;;) {
        if(redraw) {
            Begin("SERVER BROWSER","ZANDRONUM 3.2.1 / DOOMLIST.NET");
            unsigned begin=(cursor/6)*6;
            for(unsigned row=0;row<6 && begin+row<visible.size();++row) {
                auto &s=servers[visible[begin+row]];
                Card(row,ScreenText(s.name),s.address+"  "+ScreenText(s.map),begin+row==cursor,std::to_string(s.players)+"/"+std::to_string(s.maxplayers));
            }
            Text(GFX_TOP,16,221,std::to_string(visible.size())+" servers   L/R: page   A: files",muted);
            if(!error.empty()) Wrapped(GFX_TOP,16,56,error,46,11,warning);
            else if(visible.empty()) Wrapped(GFX_TOP,16,56,"No matching servers. Y changes the search; empty text shows all servers.",46,4,muted);
            Text(GFX_BOTTOM,12,16,query.empty()?"ALL SERVERS":"SEARCH: "+ScreenText(query),accent,37);
            if(!visible.empty()) {
                auto &s=servers[visible[cursor]]; std::string required,missing; std::vector<std::string> list;
                bool ready=ServerFiles(s,files,required,list,missing);
                Wrapped(GFX_BOTTOM,12,37,ScreenText(s.name),37,2);
                Text(GFX_BOTTOM,12,66,"IWAD: "+s.iwad,muted,37);
                Text(GFX_BOTTOM,12,83,std::to_string(s.wads.size())+" MOD files   "+(ready?"READY":"NEED DOWNLOAD"),ready?accent:warning,37);
                Text(GFX_BOTTOM,12,105,"START: join   A: files/download",ink,37);
                Text(GFX_BOTTOM,12,126,s.address,muted,37);
            } else Wrapped(GFX_BOTTOM,12,44,error.empty()?"Search by server name, map, address or MOD filename.":error,37,8,muted);
            Button(0,0,"Y Search",selected); Button(1,0,"SELECT Address",selected);
            Button(0,1,"X Refresh"); Button(1,1,"B Back"); Present(); redraw=false;
        }
        if(!I3DS_PumpEvents()) { StopNetwork(); return false; }
        hidScanInput(); u32 key=hidKeysDown(); key|=TouchButtons(key,KEY_Y,KEY_SELECT,KEY_X,KEY_B);
        if(Args->CheckParm("-3ds-test-browser")) {
            if(browserTestStep++==0) { Dump("ui-browser"); query="3ds zandronum test"; filter(); }
            else { Dump("ui-search"); StopNetwork(); return false; }
        }
        if(key & KEY_B) { StopNetwork(); return false; }
        if(key & KEY_X) refresh();
        if(key & KEY_Y) { std::string next=query; if(AskText("Search server, map or WAD",next)) { query=next; error.clear(); filter(); } redraw=true; }
        if(key & KEY_SELECT) {
            std::string input,address;
            if(AskText("Server address (host:port)",input)) {
                if(!DoomDirectAddress(input,address)) error="Invalid address. Use host:port (default 10666).";
                else if(!StartNetwork()) error="Cannot start network service.";
                else {
                    auto found=std::find_if(servers.begin(),servers.end(),[&](const DoomServer &s){return s.address==address;});
                    if(found!=servers.end()) { if(ConnectSelectedServer(*found,files,iwad,mods)) return true; }
                    else if(iwad.empty()) error="Select a base game IWAD in the launcher first.";
                    else {
                        connectionPassword.clear(); joinPassword.clear();
                        Args->AppendArg("-connect"); Args->AppendArg(address.c_str()); return true;
                    }
                }
            } redraw=true;
        }
        if(!visible.empty()) {
            if(key & KEY_DUP) { cursor=(cursor+visible.size()-1)%visible.size(); redraw=true; }
            if(key & KEY_DDOWN) { cursor=(cursor+1)%visible.size(); redraw=true; }
            if(key & KEY_L) { cursor=cursor>=6?cursor-6:0; redraw=true; }
            if(key & KEY_R) { cursor=std::min(cursor+6,(unsigned)visible.size()-1); redraw=true; }
            if(key & KEY_A) { auto &s=servers[visible[cursor]]; if(ServerDetails(s,files) && ConnectSelectedServer(s,files,iwad,mods)) return true; redraw=true; }
            if(key & KEY_START) { if(ConnectSelectedServer(servers[visible[cursor]],files,iwad,mods)) return true; redraw=true; }
        }
        I3DS_WaitFrame();
    }
}

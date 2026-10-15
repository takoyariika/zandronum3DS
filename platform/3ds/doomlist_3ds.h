#pragma once
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cctype>

struct DoomServer {
    std::string address, name, iwad, map, version;
    std::vector<std::string> wads;
    unsigned players=0, maxplayers=0;
    bool password=false, joinpassword=false;
};
inline std::string DoomLower(std::string value) {
    for(char &c:value) c=tolower((unsigned char)c); return value;
}
inline bool DoomServerMatches(const DoomServer &s,const std::string &query) {
    std::string q=DoomLower(query);
    if(q.empty() || DoomLower(s.name).find(q)!=std::string::npos || s.address.find(q)!=std::string::npos || DoomLower(s.map).find(q)!=std::string::npos) return true;
    for(auto &w:s.wads) if(DoomLower(w).find(q)!=std::string::npos) return true;
    return false;
}
inline bool DoomDownloadFilename(const std::string &name) {
    if(name.empty() || name.size()>240 || name[0]=='.' || name.find_first_of("/\\:")!=std::string::npos) return false;
    for(unsigned char ch:name) if(ch<32 || ch==127) return false;
    std::string lower=DoomLower(name); size_t dot=lower.rfind('.');
    return dot!=std::string::npos && (lower.substr(dot)==".wad" || lower.substr(dot)==".pk3" || lower.substr(dot)==".zip");
}
inline bool DoomDirectAddress(std::string input,std::string &address) {
    size_t start=input.find_first_not_of(" \t"),end=input.find_last_not_of(" \t");
    if(start==std::string::npos) return false;
    input=input.substr(start,end-start+1);
    size_t colon=input.find(':'); std::string host=input.substr(0,colon),port=colon==std::string::npos?"10666":input.substr(colon+1);
    if(host.empty() || host.size()>253 || port.empty() || port.size()>5) return false;
    for(unsigned char ch:host) if(!isalnum(ch) && ch!='.' && ch!='-') return false;
    for(unsigned char ch:port) if(!isdigit(ch)) return false;
    unsigned n=strtoul(port.c_str(),NULL,10); if(!n || n>65535) return false;
    address=DoomLower(host)+":"+std::to_string(n); return true;
}
inline bool DoomServerCompatible(const DoomServer &server)
{
    return server.version=="3.2.1" || server.version.compare(0,7,"3.2.1-r")==0 || server.version.compare(0,10,"3.2.1-tspg")==0 ||
           server.version.compare(0,6,"3.2.1 ")==0;
}

// Bounded JSON reader: ignores unknown API fields without constructing their
// potentially large player/history objects. Shared with host-side parser tests.
class DoomListReader {
    const std::string &data;
    size_t pos=0;
    bool valid=true;
    void ws() { while(pos<data.size() && isspace((unsigned char)data[pos])) ++pos; }
    bool eat(char c) { ws(); if(pos<data.size() && data[pos]==c) { ++pos; return true; } return false; }
    void need(char c) { if(!eat(c)) valid=false; }
    static void utf8(std::string &s,unsigned cp) {
        if(cp<128) s+=char(cp);
        else if(cp<2048) { s+=char(192|(cp>>6)); s+=char(128|(cp&63)); }
        else if(cp<65536) { s+=char(224|(cp>>12)); s+=char(128|((cp>>6)&63)); s+=char(128|(cp&63)); }
        else { s+=char(240|(cp>>18)); s+=char(128|((cp>>12)&63)); s+=char(128|((cp>>6)&63)); s+=char(128|(cp&63)); }
    }
    unsigned hex4() {
        unsigned n=0;
        for(int i=0;i<4;++i) {
            if(pos==data.size()) { valid=false; return 0; }
            char c=data[pos++]; unsigned v;
            if(c>='0' && c<='9') v=c-'0'; else if(c>='a' && c<='f') v=c-'a'+10;
            else if(c>='A' && c<='F') v=c-'A'+10; else { valid=false; return 0; }
            n=n*16+v;
        } return n;
    }
    std::string string() {
        std::string out; need('"');
        while(valid && pos<data.size()) {
            unsigned char c=data[pos++];
            if(c=='"') return out;
            if(c<32) { valid=false; break; }
            if(c=='\\') {
                if(pos==data.size()) break;
                c=data[pos++];
                if(c=='u') {
                    unsigned cp=hex4();
                    if(cp>=0xd800 && cp<=0xdbff) {
                        need('\\'); need('u'); unsigned lo=hex4();
                        if(lo<0xdc00 || lo>0xdfff) valid=false;
                        cp=0x10000+((cp-0xd800)<<10)+(lo-0xdc00);
                    } else if(cp>=0xdc00 && cp<=0xdfff) valid=false;
                    utf8(out,cp); continue;
                }
                switch(c) {
                    case 'n': c='\n'; break; case 'r': c='\r'; break; case 't': c='\t'; break;
                    case 'b': c='\b'; break; case 'f': c='\f'; break;
                    case '"': case '\\': case '/': break; default: valid=false;
                }
            }
            out+=char(c);
            if(out.size()>8192) { valid=false; break; }
        } valid=false; return out;
    }
    std::string scalar() {
        ws(); size_t begin=pos;
        while(pos<data.size() && data[pos]!=',' && data[pos]!=']' && data[pos]!='}' && !isspace((unsigned char)data[pos])) ++pos;
        std::string s=data.substr(begin,pos-begin);
        if(s!="true" && s!="false" && s!="null") {
            char *end; std::strtod(s.c_str(),&end);
            if(s.empty() || *end) valid=false;
        } return s;
    }
    void skip(unsigned depth=0) {
        ws(); if(depth>32 || pos==data.size()) { valid=false; return; }
        if(data[pos]=='"') { string(); return; }
        if(eat('{')) {
            if(eat('}')) return;
            do { string(); need(':'); skip(depth+1); } while(valid && eat(',')); need('}');
        } else if(eat('[')) {
            if(eat(']')) return;
            do { skip(depth+1); } while(valid && eat(',')); need(']');
        } else scalar();
    }
    void server(DoomServer &s) {
        need('{'); if(eat('}')) return;
        do {
            std::string key=string(); need(':');
            if(key=="name") s.name=string();
            else if(key=="iwad") s.iwad=string();
            else if(key=="mapname") s.map=string();
            else if(key=="version") s.version=string();
            else if(key=="numplayers") s.players=std::strtoul(scalar().c_str(),NULL,10);
            else if(key=="maxclients") s.maxplayers=std::strtoul(scalar().c_str(),NULL,10);
            else if(key=="forcepassword") s.password=scalar()=="true";
            else if(key=="forcejoinpassword") s.joinpassword=scalar()=="true";
            else if(key=="pwads") {
                need('['); if(!eat(']')) {
                    do { s.wads.push_back(string()); if(s.wads.size()>128) valid=false; } while(valid && eat(',')); need(']');
                }
            } else skip();
        } while(valid && eat(',')); need('}');
    }
public:
    explicit DoomListReader(const std::string &d):data(d) {}
    bool read(std::vector<DoomServer> &out) {
        out.clear(); if(data.size()>2*1024*1024) return false;
        need('{'); if(!eat('}')) {
            do {
                DoomServer s; s.address=string(); need(':'); server(s);
                unsigned a,b,c,d,p; char extra;
                bool ip=sscanf(s.address.c_str(),"%u.%u.%u.%u:%u%c",&a,&b,&c,&d,&p,&extra)==5 && a<256 && b<256 && c<256 && d<256 && p>0 && p<65536;
                if(valid && ip && !s.name.empty() && !s.iwad.empty()) out.push_back(s);
                if(out.size()>2048) valid=false;
            } while(valid && eat(',')); need('}');
        }
        ws(); if(!valid || pos!=data.size()) { out.clear(); return false; } return true;
    }
};

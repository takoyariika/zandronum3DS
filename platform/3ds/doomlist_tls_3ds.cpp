#include <3ds.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <poll.h>
#include <string>
#include <vector>
#include "mbedtls/ssl.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "doomlist_ca_3ds.h"
#include "doomlist_http_3ds.h"
#include "doomlist_3ds.h"
#include "i_3ds.h"
#include "m_argv.h"
#include "mbedtls/platform_time.h"
extern "C" mbedtls_ms_time_t mbedtls_ms_time(void) { return (mbedtls_ms_time_t)osGetTime(); }

namespace {
Result randomResult;
bool transferCancelled;
unsigned downloadConnections=1;
u64 NetworkMillis() { return svcGetSystemTick()/(SYSCLOCK_ARM11/1000); }
int Random(void *,unsigned char *out,size_t length)
{
    while(length) {
        size_t n=length>32?32:length;
        randomResult=PS_GenerateRandomBytes(out,n);
        if(R_FAILED(randomResult)) return MBEDTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED;
        out+=n; length-=n;
    } return 0;
}
mbedtls_time_t BrowserTime(mbedtls_time_t *out)
{
    // Use libctru's documented 1900 epoch, independently of newlib's epoch.
    mbedtls_time_t now=(mbedtls_time_t)(osGetTime()/1000-2208988800ULL);
    // Older Citra's RTC reports 2056 against the current libctru. The replay
    // supplies host UTC explicitly; normal console launches always use RTC.
    if(Args && Args->CheckParm("-3ds-test-http")) {
        if(const char *utc=Args->CheckValue("-3ds-test-utc")) now=(mbedtls_time_t)strtoll(utc,NULL,10);
    }
    if(out) *out=now; return now;
}
int Send(void *context,const unsigned char *data,size_t size)
{
    int n=send(*(int*)context,data,size,0);
    if(n>=0) return n;
    return errno==EAGAIN || errno==EWOULDBLOCK || errno==EINTR || errno==EINPROGRESS || errno==EALREADY || errno==ENOTCONN ? MBEDTLS_ERR_SSL_WANT_WRITE : MBEDTLS_ERR_NET_SEND_FAILED;
}
int Receive(void *context,unsigned char *data,size_t size)
{
    int n=recv(*(int*)context,data,size,0);
    if(n>=0) return n;
    return errno==EAGAIN || errno==EWOULDBLOCK || errno==EINTR ? MBEDTLS_ERR_SSL_WANT_READ : MBEDTLS_ERR_NET_RECV_FAILED;
}
bool Retry(u64 deadline)
{
    if(NetworkMillis()>=deadline) return false;
    if(!I3DS_PumpEvents()) { transferCancelled=true; return false; }
    hidScanInput(); if(hidKeysDown() & KEY_B) { transferCancelled=true; return false; }
    svcSleepThread(1000000); return true;
}
bool DownloadEvents()
{
    if(!I3DS_PumpEvents()) { transferCancelled=true; return false; }
    hidScanInput(); u32 keys=hidKeysDown();
    if(keys & KEY_TOUCH) { touchPosition p; hidTouchRead(&p); if(p.px>=8 && p.px<156 && p.py>=203 && p.py<231) keys|=KEY_B; }
    if(keys & KEY_B) { transferCancelled=true; return false; }
    return true;
}
struct Connection {
    int socket=-1;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config config;
    mbedtls_x509_crt ca;
    mbedtls_ctr_drbg_context rng;
    Connection() { mbedtls_ssl_init(&ssl); mbedtls_ssl_config_init(&config); mbedtls_x509_crt_init(&ca); mbedtls_ctr_drbg_init(&rng); }
    ~Connection() { if(socket>=0) close(socket); mbedtls_ssl_free(&ssl); mbedtls_ssl_config_free(&config); mbedtls_x509_crt_free(&ca); mbedtls_ctr_drbg_free(&rng); }
};
bool Open(Connection &c,const char *hostname,std::string &error,const mbedtls_ssl_session *session=NULL)
{
    // Include SHA-384 support for the current Let's Encrypt YE certificate chain.
    u64 deadline;
    mbedtls_platform_set_time(BrowserTime);
    int rc=mbedtls_ctr_drbg_seed(&c.rng,Random,NULL,(const unsigned char*)"Zandronum3DS",12);
    if(!rc) rc=mbedtls_x509_crt_parse_der(&c.ca,doomlistRootCA,sizeof(doomlistRootCA));
    if(!rc) rc=mbedtls_x509_crt_parse_der(&c.ca,doomlistRootX2,sizeof(doomlistRootX2));
    if(!rc) rc=mbedtls_ssl_config_defaults(&c.config,MBEDTLS_SSL_IS_CLIENT,MBEDTLS_SSL_TRANSPORT_STREAM,MBEDTLS_SSL_PRESET_DEFAULT);
    if(!rc) {
        mbedtls_ssl_conf_authmode(&c.config,MBEDTLS_SSL_VERIFY_REQUIRED);
        mbedtls_ssl_conf_ca_chain(&c.config,&c.ca,NULL);
        mbedtls_ssl_conf_rng(&c.config,mbedtls_ctr_drbg_random,&c.rng);
        mbedtls_ssl_conf_min_tls_version(&c.config,MBEDTLS_SSL_VERSION_TLS1_2);
        rc=mbedtls_ssl_setup(&c.ssl,&c.config);
    }
    if(!rc) rc=mbedtls_ssl_set_hostname(&c.ssl,hostname);
    if(!rc && session) rc=mbedtls_ssl_set_session(&c.ssl,session);
    if(rc) { char message[128]; snprintf(message,sizeof(message),"TLS init -%04x; RNG %08lx",-rc,(unsigned long)randomResult); error=message; return false; }
    hostent *host=gethostbyname(hostname);
    if(!host || host->h_addrtype!=AF_INET || host->h_length!=4) { error="DNS failed; check Wi-Fi"; return false; }
    c.socket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(c.socket<0) { error="TCP socket unavailable"; return false; }
    int nonblocking=1;
    if(ioctl(c.socket,FIONBIO,&nonblocking)<0) { error="Cannot configure TCP socket"; return false; }
    sockaddr_in addr={}; addr.sin_family=AF_INET; addr.sin_port=htons(443); memcpy(&addr.sin_addr,host->h_addr_list[0],4);
    deadline=NetworkMillis()+30000;
    rc=connect(c.socket,(sockaddr*)&addr,sizeof(addr));
    if(rc<0 && errno!=EINPROGRESS && errno!=EWOULDBLOCK && errno!=EAGAIN) { error="TCP connect errno="+std::to_string(errno); return false; }
    for(;;) {
        // Hardware SOC uses 0x08 for write readiness, while older libctru/
        // Citra use 0x10. Request both; never treat 0x08 as an error here.
        // getpeername alone can succeed before TCP establishment in Citra.
        pollfd pending={c.socket,0x18,0};
        int ready=poll(&pending,1,0);
        if(ready<0) { error="TCP poll errno="+std::to_string(errno); return false; }
        if(ready>0) {
            // A failed connection can also wake poll. Older Citra converts an
            // uninitialized peer address on getpeername failure and aborts.
            // Reject terminal socket errors first, but allow SOC's stale -26
            // (in-progress) even after a successful connection on hardware.
            int socketError=0; socklen_t size=sizeof(socketError);
            if(!getsockopt(c.socket,SOL_SOCKET,SO_ERROR,&socketError,&size) && socketError) {
                int e=socketError<0?-socketError:socketError;
                if(socketError!=-26 && e!=EINPROGRESS && e!=EWOULDBLOCK && e!=EAGAIN && e!=EALREADY && e!=EINTR && e!=ENOTCONN) {
                    error="TCP failed: "+std::string(hostname)+" socket error="+std::to_string(socketError); return false;
                }
            }
        }
        sockaddr_in peer={}; socklen_t len=sizeof(peer);
        if(ready>0 && getpeername(c.socket,(sockaddr*)&peer,&len)==0) break;
        int peerError=errno;
        if(!Retry(deadline)) {
            char address[INET_ADDRSTRLEN]={}; inet_ntop(AF_INET,&addr.sin_addr,address,sizeof(address));
            error="TCP wait timeout/cancel: "+std::string(address)+" peer errno="+std::to_string(peerError);
            return false;
        }
    }
    deadline=NetworkMillis()+60000;
    mbedtls_ssl_set_bio(&c.ssl,&c.socket,Send,Receive,NULL);
    while((rc=mbedtls_ssl_handshake(&c.ssl))) {
        if((rc!=MBEDTLS_ERR_SSL_WANT_READ && rc!=MBEDTLS_ERR_SSL_WANT_WRITE) || !Retry(deadline)) {
            char message[128]; snprintf(message,sizeof(message),"TLS -%04x flags=%lx; check clock/Wi-Fi",-rc,(unsigned long)mbedtls_ssl_get_verify_result(&c.ssl)); error=message; return false;
        }
    }
    return true;
}
bool Request(Connection &c,const char *hostname,const std::string &path,std::string &error,const std::string &extra="")
{
    std::string request="GET "+path+" HTTP/1.1\r\nHost: "+hostname+"\r\nUser-Agent: Zandronum3DS/0.1.18\r\nAccept-Encoding: identity\r\n"+extra+"Connection: close\r\n\r\n";
    u64 deadline=NetworkMillis()+30000;
    size_t sent=0;
    while(sent<request.size()) {
        int rc=mbedtls_ssl_write(&c.ssl,(const unsigned char*)request.data()+sent,request.size()-sent);
        if(rc>0) sent+=rc;
        else if((rc!=MBEDTLS_ERR_SSL_WANT_READ && rc!=MBEDTLS_ERR_SSL_WANT_WRITE) || !Retry(deadline)) { error="HTTPS send failed"; return false; }
    }
    return true;
}
bool Transfer(std::string &json,std::string &error)
{
    Connection c;
    if(!Open(c,"doomlist.net",error) || !Request(c,"doomlist.net","/api/full",error)) return false;
    std::string response; unsigned char buffer[4096];
    const u64 readLimit=NetworkMillis()+120000;
    u64 deadline=NetworkMillis()+30000;
    for(;;) {
        if(NetworkMillis()>=readLimit) { error="HTTPS receive total timeout; bytes="+std::to_string(response.size()); return false; }
        int rc=mbedtls_ssl_read(&c.ssl,buffer,sizeof(buffer));
        if(rc>0) {
            if(response.size()+rc>2*1024*1024+65536) { error="Server list too large"; return false; }
            response.append((const char*)buffer,rc);
            deadline=NetworkMillis()+30000;
            // A complete framed response does not need to wait for TCP close.
            if(response.find("\r\n\r\n")!=std::string::npos && DoomListHttpBody(response,json)) {
                std::string header=response.substr(0,response.find("\r\n\r\n"));
                for(char &ch:header) ch=tolower((unsigned char)ch);
                if(header.find("\r\ncontent-length:")!=std::string::npos || header.find("\r\ntransfer-encoding: chunked")!=std::string::npos) return true;
            }
        } else if(rc==0 || rc==MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) break;
        else if((rc!=MBEDTLS_ERR_SSL_WANT_READ && rc!=MBEDTLS_ERR_SSL_WANT_WRITE) || !Retry(deadline)) { error="HTTPS receive failed/timeout: rc="+std::to_string(rc)+" bytes="+std::to_string(response.size()); return false; }
    }
    if(!DoomListHttpBody(response,json)) { error="Invalid HTTP server-list response"; return false; }
    return true;
}

std::string HeaderField(const std::string &header,const char *name)
{
    std::string lower=header;
    for(char &ch:lower) ch=tolower((unsigned char)ch);
    size_t pos=lower.find(std::string("\r\n")+name+":");
    if(pos==std::string::npos) return "";
    pos=header.find(':',pos)+1;
    size_t end=header.find("\r\n",pos);
    std::string value=header.substr(pos,end==std::string::npos?end:end-pos);
    size_t first=value.find_first_not_of(" \t"),last=value.find_last_not_of(" \t");
    return first==std::string::npos?"":value.substr(first,last-first+1);
}
// Two independent nonblocking TLS streams, serviced on the UI thread. No
// worker touches APT/HID, and buffers remain bounded independently of WAD size.
// 0 means range support was unavailable before any file data was written.
int DownloadParallel(Connection &first,const char *host,const std::string &path,FILE *file,
                     const std::string &fields,const unsigned char *initial,size_t initialSize,
                     size_t expected,std::string &error,void (*progress)(size_t,size_t))
{
    if(expected<2*1024*1024 || HeaderField(fields,"accept-ranges")!="bytes" ||
       (Args && Args->CheckParm("-3ds-download-single"))) return 0;
    std::string validator=HeaderField(fields,"etag"),kind="etag";
    if(validator.empty() || validator.compare(0,2,"W/")==0) { validator=HeaderField(fields,"last-modified"); kind="last-modified"; }
    if(validator.empty() || validator.find_first_of("\r\n")!=std::string::npos) return 0;
    const size_t split=expected/2;
    Connection second;
    struct Session {
        mbedtls_ssl_session value;
        Session() { mbedtls_ssl_session_init(&value); }
        ~Session() { mbedtls_ssl_session_free(&value); }
    } saved;
    // Only resume the just-verified TLS session for this exact host. This
    // avoids paying for another certificate/ECDHE exchange where supported.
    bool haveSession=mbedtls_ssl_get_session(&first.ssl,&saved.value)==0;
    std::string range="Range: bytes="+std::to_string(split)+"-"+std::to_string(expected-1)+"\r\nIf-Range: "+validator+"\r\n";
    if(!Open(second,host,error,haveSession?&saved.value:NULL) || !Request(second,host,path,error,range)) return transferCancelled?-1:0;
    std::string header; unsigned char buffer[16384];
    u64 deadline=NetworkMillis()+30000;
    size_t body=0;
    for(;;) {
        int rc=mbedtls_ssl_read(&second.ssl,buffer,sizeof(buffer));
        if(rc>0) {
            header.append((const char*)buffer,rc);
            size_t end=header.find("\r\n\r\n");
            if(end!=std::string::npos) { body=end+4; break; }
            if(header.size()>65536) return 0;
        } else if((rc!=MBEDTLS_ERR_SSL_WANT_READ && rc!=MBEDTLS_ERR_SSL_WANT_WRITE) || !Retry(deadline)) return transferCancelled?-1:0;
    }
    unsigned long a=0,b=0,n=0;
    std::string contentRange=HeaderField(header,"content-range"),length=HeaderField(header,"content-length");
    char trailing=0;
    if(body>65540 || (header.compare(0,13,"HTTP/1.1 206 ") && header.compare(0,13,"HTTP/1.0 206 ")) ||
       sscanf(contentRange.c_str(),"bytes %lu-%lu/%lu%c",&a,&b,&n,&trailing)!=3 ||
       a!=split || b!=expected-1 || n!=expected || length!=std::to_string(expected-split) ||
       !HeaderField(header,"transfer-encoding").empty() || HeaderField(header,kind.c_str())!=validator) return 0;
    downloadConnections=2;
    struct Piece {
        Connection *connection;
        size_t start,length,received=0,written=0;
        std::vector<unsigned char> pending;
        u64 idle;
        Piece(Connection *c,size_t s,size_t len):connection(c),start(s),length(len),idle(NetworkMillis()+30000) { pending.reserve(128*1024+16384); }
    } parts[]={Piece(&first,0,split),Piece(&second,split,expected-split)};
    auto append=[&](Piece &p,const unsigned char *data,size_t size) {
        if(size>p.length-p.received) size=p.length-p.received;
        p.pending.insert(p.pending.end(),data,data+size); p.received+=size;
    };
    append(parts[0],initial,initialSize);
    if(header.size()-body>parts[1].length) { error="Range body exceeds size"; return -1; }
    append(parts[1],(const unsigned char*)header.data()+body,header.size()-body);
    const u64 total=NetworkMillis()+1800000;
    u64 lastProgress=0,lastEvents=0;
    while(parts[0].written+parts[1].written<expected) {
        bool advanced=false;
        for(Piece &p:parts) {
            if(p.received<p.length) {
                int rc=mbedtls_ssl_read(&p.connection->ssl,buffer,sizeof(buffer));
                if(rc>0) {
                    // The first full response intentionally stops at the split;
                    // the second response must contain only its requested range.
                    if(p.start && (size_t)rc>p.length-p.received) { error="Range exceeds advertised size"; return -1; }
                    append(p,buffer,rc); p.idle=NetworkMillis()+30000; advanced=true;
                } else if(rc!=MBEDTLS_ERR_SSL_WANT_READ && rc!=MBEDTLS_ERR_SSL_WANT_WRITE) { error="Incomplete parallel download: "+std::to_string(rc); return -1; }
                else if(NetworkMillis()>=p.idle) { error="Parallel download idle timeout"; return -1; }
            }
            if(!p.pending.empty() && (p.pending.size()>=128*1024 || p.received==p.length)) {
                if(fseek(file,(long)(p.start+p.written),SEEK_SET) || fwrite(p.pending.data(),1,p.pending.size(),file)!=p.pending.size()) { error="SD write failed; check free space"; return -1; }
                p.written+=p.pending.size(); p.pending.clear();
            }
        }
        u64 now=NetworkMillis(); size_t received=parts[0].received+parts[1].received;
        if(now>=total) { error="Parallel download total timeout"; return -1; }
        if(now-lastEvents>=30) { if(!DownloadEvents()) { error="Download cancelled"; return -1; } lastEvents=now; }
        if(progress && (now-lastProgress>=500 || received==expected)) { progress(received,expected); lastProgress=now; }
        if(!advanced && received<expected) svcSleepThread(1000000);
    }
    error.clear(); return 1;
}
bool Download(const char *hostname,const std::string &path,FILE *file,std::string &error,
              void (*progress)(size_t,size_t),unsigned redirects=0)
{
    Connection c;
    if(!Open(c,hostname,error) || !Request(c,hostname,path,error)) return false;
    std::string header; unsigned char buffer[16384];
    size_t received=0,expected=0; bool gotHeader=false;
    u64 idle=NetworkMillis()+30000,total=NetworkMillis()+1800000,lastProgress=0,lastEvents=0;
    for(;;) {
        if(NetworkMillis()>=total) { error="Download total timeout"; return false; }
        int rc=mbedtls_ssl_read(&c.ssl,buffer,sizeof(buffer));
        if(rc>0) {
            idle=NetworkMillis()+30000;
            const unsigned char *data=buffer; size_t count=rc;
            if(!gotHeader) {
                header.append((const char*)buffer,rc);
                size_t end=header.find("\r\n\r\n");
                if(end==std::string::npos) { if(header.size()>65536) { error="Download headers too large"; return false; } continue; }
                if(end>65536) { error="Download headers too large"; return false; }
                if(header.compare(0,13,"HTTP/1.1 200 ") && header.compare(0,13,"HTTP/1.0 200 ")) {
                    std::string fields=header.substr(0,end),lower=fields;
                    for(char &ch:lower) ch=tolower((unsigned char)ch);
                    int status=0; sscanf(header.c_str(),"HTTP/%*s %d",&status);
                    size_t location=lower.find("\r\nlocation:");
                    if(redirects<5 && location!=std::string::npos && (status==301 || status==302 || status==303 || status==307 || status==308)) {
                        size_t begin=location+11,finish=fields.find("\r\n",begin);
                        std::string target=fields.substr(begin,finish==std::string::npos?finish:finish-begin);
                        size_t first=target.find_first_not_of(" \t"),last=target.find_last_not_of(" \t");
                        if(first!=std::string::npos) target=target.substr(first,last-first+1);
                        std::string host=hostname,resource=target;
                        if(target.compare(0,8,"https://")==0) {
                            size_t slash=target.find('/',8); host=target.substr(8,slash==std::string::npos?slash:slash-8);
                            resource=slash==std::string::npos?"/":target.substr(slash);
                        }
                        bool allowed=host=="allfearthesentinel.com" || host=="static.allfearthesentinel.com" || host=="www.ifocserv.net" || host=="ifocserv.net" || host=="wads.pizza-doom.it";
                        if(allowed && !resource.empty() && resource[0]=='/' && resource.compare(0,2,"//") && resource.find_first_of("\r\n ") == std::string::npos)
                            return Download(host.c_str(),resource,file,error,progress,redirects+1);
                        error="Unsupported download redirect: "+target; return false;
                    }
                    error="Download "+header.substr(0,header.find("\r\n")); return false;
                }
                std::string fields=header.substr(0,end);
                for(char &ch:fields) ch=tolower((unsigned char)ch);
                size_t length=fields.find("\r\ncontent-length:");
                if(length==std::string::npos || fields.find("\r\ntransfer-encoding:")!=std::string::npos) { error="Download missing fixed length"; return false; }
                const char *number=fields.c_str()+length+17; char *tail;
                unsigned long long n=strtoull(number,&tail,10);
                while(*tail==' ' || *tail=='\t') ++tail;
                if(tail==number || (*tail && *tail!='\r') || n<4 || n>512ULL*1024*1024) { error="Invalid download size (max 512 MiB)"; return false; }
                expected=(size_t)n; gotHeader=true;
                data=(const unsigned char*)header.data()+end+4; count=header.size()-end-4;
                std::string originalFields=header.substr(0,end);
                int parallel=DownloadParallel(c,hostname,path,file,originalFields,data,count,expected,error,progress);
                if(parallel) return parallel>0;
                error.clear(); idle=NetworkMillis()+30000;
            }
            if(count>expected-received) { error="Download exceeds advertised size"; return false; }
            if(fwrite(data,1,count,file)!=count) { error="SD write failed; check free space"; return false; }
            received+=count;
            if(progress && (NetworkMillis()-lastProgress>=500 || received==expected)) { progress(received,expected); lastProgress=NetworkMillis(); }
            if(gotHeader) header.clear();
            if(received==expected) return true;
        } else if(rc==0 || rc==MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) { error="Incomplete download: "+std::to_string(received)+"/"+std::to_string(expected); return false; }
        else if((rc!=MBEDTLS_ERR_SSL_WANT_READ && rc!=MBEDTLS_ERR_SSL_WANT_WRITE) || !Retry(idle)) { error="Download failed/cancelled: "+std::to_string(rc); return false; }
        if(NetworkMillis()-lastEvents>=30) {
            if(!DownloadEvents()) { error="Download cancelled"; return false; }
            lastEvents=NetworkMillis();
        }
    }
}
}
unsigned I3DS_DownloadConnections() { return downloadConnections; }
bool I3DS_FetchDoomList(std::string &json,std::string &error)
{
    if(!I3DS_NetworkEnabled()) { error="Select Network mode first"; return false; }
    randomResult=0; transferCancelled=false;
    Result rc=psInit();
    bool ok=false;
    if(R_FAILED(rc)) { char message[96]; snprintf(message,sizeof(message),"PS service failed: %08lx",(unsigned long)rc); error=message; }
    else { ok=Transfer(json,error); psExit(); }
    // One report per user-requested refresh; no logging during gameplay.
    FILE *report=fopen("sdmc:/3ds/zandronum/network.txt","w");
    if(report) {
        time_t utc=(time_t)BrowserTime(NULL); tm *date=gmtime(&utc); char text[80]={};
        if(date) strftime(text,sizeof(text),"%Y-%m-%d %H:%M:%S",date);
        fprintf(report,"doomlist.net: %s\n%s\nRTC UTC: %s (%lld)\nPS: %08lx RNG: %08lx\nJSON bytes: %u\n",ok?"OK":"FAILED",error.c_str(),text,(long long)utc,(unsigned long)rc,(unsigned long)randomResult,(unsigned)json.size());
        fclose(report);
    }
    return ok;
}

bool I3DS_DownloadWad(const std::string &name,const std::string &destination,
                     std::string &error,void (*progress)(size_t,size_t))
{
    if(!I3DS_NetworkEnabled()) { error="Select Network mode first"; return false; }
    if(!DoomDownloadFilename(name)) { error="Invalid WAD filename"; return false; }
    struct stat existing;
    if(!stat(destination.c_str(),&existing)) { error.clear(); return true; }
    std::string encoded;
    for(unsigned char ch:name) {
        if(isalnum(ch) || ch=='-' || ch=='_' || ch=='.') encoded+=char(ch);
        else { char hex[4]; snprintf(hex,sizeof(hex),"%%%02X",ch); encoded+=hex; }
    }
    Result rc=psInit();
    if(R_FAILED(rc)) { error="Download RNG service failed"; return false; }
    const char *hosts[]={"allfearthesentinel.com","static.allfearthesentinel.com","www.ifocserv.net","wads.pizza-doom.it"};
    const char *paths[]={"/zandronum/download.php?file=","/wads/","/ftp/wads/","/"};
    // The TSPG PHP/static endpoints reject large PK3s with403. Try the public
    // archive mirror first for PK3/ZIP; keep the server's WAD endpoint first.
    unsigned sites[]={0,1,2,3};
    std::string lower=DoomLower(name);
    if(lower.size()>=4 && (lower.compare(lower.size()-4,4,".pk3")==0 || lower.compare(lower.size()-4,4,".zip")==0)) { sites[0]=2; sites[1]=0; sites[2]=1; }
    const std::string partial=destination+".part"; bool ok=false;
    transferCancelled=false; downloadConnections=1;
    FILE *report=fopen("sdmc:/3ds/zandronum/download.txt","w");
    if(report) { fprintf(report,"File: %s\n",name.c_str()); fflush(report); }
    for(unsigned attempt=0;attempt<4;++attempt) {
        unsigned site=sites[attempt]; downloadConnections=1;
        error.clear();
        FILE *file=fopen(partial.c_str(),"wb");
        if(!file) { error="Cannot create file on SD"; break; }
        // newlib's tiny default buffer otherwise causes many SD IPC writes.
        std::vector<char> diskBuffer(64*1024);
        setvbuf(file,diskBuffer.data(),_IOFBF,diskBuffer.size());
        u64 started=NetworkMillis();
        ok=Download(hosts[site],paths[site]+encoded,file,error,progress);
        if(fclose(file)) { error="Cannot finish SD file"; ok=false; }
        if(ok) {
            file=fopen(partial.c_str(),"rb"); unsigned char magic[4]={};
            if(file) { fread(magic,1,4,file); fclose(file); }
            // Commercial IWADs are supplied by the owner, never fetched as mods.
            ok=!memcmp(magic,"PWAD",4) || !memcmp(magic,"PK\3\4",4);
            if(!ok) error="Downloaded file is not a PWAD/PK3";
        }
        if(report) { fprintf(report,"%s: %s %s (%llu ms; connections=%u)\n",hosts[site],ok?"OK":"FAILED",error.c_str(),(unsigned long long)(NetworkMillis()-started),downloadConnections); fflush(report); }
        if(ok || transferCancelled || error.find("SD")!=std::string::npos) break;
    }
    psExit();
    if(ok && rename(partial.c_str(),destination.c_str())) { error="Cannot install downloaded WAD"; ok=false; }
    if(!ok) remove(partial.c_str());
    else error.clear();
    if(report) { fprintf(report,"Installed: %s\n",ok?"yes":"no"); fclose(report); }
    return ok;
}

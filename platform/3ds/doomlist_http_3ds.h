#pragma once
#include <string>
#include <cstdlib>
#include <cctype>

inline bool DoomListHttpBody(const std::string &response,std::string &body)
{
    const size_t limit=2*1024*1024;
    body.clear();
    if(response.size()>limit+65536) return false;
    size_t header=response.find("\r\n\r\n");
    if(header==std::string::npos || header>65536 ||
       (response.compare(0,13,"HTTP/1.1 200 ") && response.compare(0,13,"HTTP/1.0 200 "))) return false;
    std::string fields=response.substr(0,header);
    for(char &c:fields) c=tolower((unsigned char)c);
    size_t start=header+4;
    if(fields.find("\r\ntransfer-encoding: chunked")!=std::string::npos) {
        while(start<response.size()) {
            size_t end=response.find("\r\n",start);
            if(end==std::string::npos || end-start>128) return false;
            std::string number=response.substr(start,end-start); char *tail;
            unsigned long n=strtoul(number.c_str(),&tail,16);
            if(tail==number.c_str() || (*tail && *tail!=';') || n>limit-body.size()) return false;
            start=end+2;
            if(!n) return true;
            if(n>response.size()-start || response.compare(start+n,2,"\r\n")) return false;
            body.append(response,start,n); start+=n+2;
        } return false;
    }
    body=response.substr(start);
    size_t length=fields.find("\r\ncontent-length:");
    if(length!=std::string::npos) {
        char *tail; const char *number=fields.c_str()+length+17;
        unsigned long n=strtoul(number,&tail,10);
        while(*tail==' ' || *tail=='\t') ++tail;
        if(tail==number || (*tail && *tail!='\r') || n!=body.size()) return false;
    }
    return body.size()<=limit;
}

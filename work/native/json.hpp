#pragma once
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <cstdint>
#include <limits>

// Small UTF-8 JSON codec. Integers are retained exactly for file sizes.
namespace sma {
struct Json {
    enum Kind { Null, Boolean, Number, String, Array, Object } kind=Null;
    bool flag=false; int64_t number=0; std::string text,decimal;
    std::vector<Json> items; std::map<std::string,Json> fields;
    Json()=default; Json(bool b):kind(Boolean),flag(b){} Json(int64_t n):kind(Number),number(n){}
    Json(const std::string& s):kind(String),text(s){} Json(const char* s):Json(std::string(s)){}
    static Json object(){Json j;j.kind=Object;return j;} static Json array(){Json j;j.kind=Array;return j;}
    Json& operator[](const std::string& k){if(kind==Null)kind=Object;if(kind!=Object)throw std::runtime_error("JSON object expected");return fields[k];}
    const Json& get(const std::string& k)const{static const Json empty;auto i=fields.find(k);return i==fields.end()?empty:i->second;}
    std::string str(const std::string& fallback="")const{return kind==String?text:fallback;}
    bool boolean(bool fallback=false)const{return kind==Boolean?flag:fallback;}
    int64_t integer(int64_t fallback=0)const{return kind==Number&&decimal.empty()?number:fallback;}
    bool has(const std::string& k)const{return fields.count(k)!=0;}
    static void utf8(std::string& s,uint32_t c){
        if(c<=0x7f)s+=char(c);else if(c<=0x7ff){s+=char(0xc0|(c>>6));s+=char(0x80|(c&63));}
        else if(c<=0xffff){s+=char(0xe0|(c>>12));s+=char(0x80|((c>>6)&63));s+=char(0x80|(c&63));}
        else if(c<=0x10ffff){s+=char(0xf0|(c>>18));s+=char(0x80|((c>>12)&63));s+=char(0x80|((c>>6)&63));s+=char(0x80|(c&63));}else throw std::runtime_error("Invalid Unicode");
    }
    struct Parser {
        const std::string& s;size_t p=0;unsigned depth=0;
        explicit Parser(const std::string& in):s(in){if(s.size()>=3&&s.substr(0,3)=="\xef\xbb\xbf")p=3;}
        [[noreturn]] void fail(){throw std::runtime_error("Invalid JSON at "+std::to_string(p));}
        void ws(){while(p<s.size()&&(s[p]==' '||s[p]=='\t'||s[p]=='\r'||s[p]=='\n'))++p;}
        bool take(char c){ws();if(p<s.size()&&s[p]==c){++p;return true;}return false;}
        uint32_t hex(){if(p+4>s.size())fail();uint32_t n=0;for(int i=0;i<4;++i){char c=s[p++];n*=16;if(c>='0'&&c<='9')n+=c-'0';else if(c>='a'&&c<='f')n+=c-'a'+10;else if(c>='A'&&c<='F')n+=c-'A'+10;else fail();}return n;}
        std::string string(){
            if(!take('"'))fail();std::string out;
            while(p<s.size()){
                unsigned char c=s[p++];if(c=='"')return out;if(c<32)fail();
                if(c!='\\'){out+=char(c);continue;}if(p==s.size())fail();c=s[p++];
                switch(c){case '"':case '\\':case '/':out+=char(c);break;case 'b':out+='\b';break;case 'f':out+='\f';break;case 'n':out+='\n';break;case 'r':out+='\r';break;case 't':out+='\t';break;
                case 'u':{auto cp=hex();if(cp>=0xd800&&cp<=0xdbff){if(p+2>s.size()||s[p++]!='\\'||s[p++]!='u')fail();auto lo=hex();if(lo<0xdc00||lo>0xdfff)fail();cp=0x10000+((cp-0xd800)<<10)+(lo-0xdc00);}else if(cp>=0xdc00&&cp<=0xdfff)fail();Json::utf8(out,cp);break;}default:fail();}
            }fail();
        }
        Json value(){
            ws();if(p>=s.size()||++depth>96)fail();Json j;
            if(s[p]=='"')j=Json(string());
            else if(take('{')){j=Json::object();if(!take('}')){do{auto k=string();if(!take(':')||j.has(k))fail();j[k]=value();}while(take(','));if(!take('}'))fail();}}
            else if(take('[')){j=Json::array();if(!take(']')){do{j.items.push_back(value());}while(take(','));if(!take(']'))fail();}}
            else if(s.compare(p,4,"true")==0){p+=4;j=Json(true);}else if(s.compare(p,5,"false")==0){p+=5;j=Json(false);}else if(s.compare(p,4,"null")==0){p+=4;}
            else{
                size_t start=p;if(s[p]=='-')++p;if(p>=s.size()||s[p]<'0'||s[p]>'9')fail();if(s[p]=='0')++p;else while(p<s.size()&&s[p]>='0'&&s[p]<='9')++p;
                bool fractional=false;if(p<s.size()&&s[p]=='.'){fractional=true;++p;size_t digits=p;while(p<s.size()&&s[p]>='0'&&s[p]<='9')++p;if(p==digits)fail();}
                if(p<s.size()&&(s[p]=='e'||s[p]=='E')){fractional=true;++p;if(p<s.size()&&(s[p]=='+'||s[p]=='-'))++p;size_t digits=p;while(p<s.size()&&s[p]>='0'&&s[p]<='9')++p;if(p==digits)fail();}
                auto token=s.substr(start,p-start);if(fractional){j.kind=Number;j.decimal=token;}else try{j=Json(int64_t(std::stoll(token)));}catch(...){fail();}
            }--depth;return j;
        }
    };
    static Json parse(const std::string& s){Parser p(s);auto j=p.value();p.ws();if(p.p!=s.size())p.fail();return j;}
    static std::string quote(const std::string& s){std::string out="\"";const char* hex="0123456789abcdef";for(unsigned char c:s){switch(c){case '"':out+="\\\"";break;case '\\':out+="\\\\";break;case '\n':out+="\\n";break;case '\r':out+="\\r";break;case '\t':out+="\\t";break;default:if(c<32){out+="\\u00";out+=hex[c>>4];out+=hex[c&15];}else out+=char(c);}}return out+'"';}
    std::string dump()const{
        switch(kind){case Null:return "null";case Boolean:return flag?"true":"false";case Number:return decimal.empty()?std::to_string(number):decimal;case String:return quote(text);case Array:{std::string s="[";for(size_t i=0;i<items.size();++i){if(i)s+=',';s+=items[i].dump();}return s+"]";}case Object:{std::string s="{";bool first=true;for(auto& kv:fields){if(!first)s+=',';first=false;s+=quote(kv.first)+":"+kv.second.dump();}return s+"}";}}throw std::runtime_error("JSON kind");
    }
};
}

#pragma once
#include "platform.hpp"
namespace sma {
inline fs::path full(const fs::path& p){return fs::absolute(p).lexically_normal();}
inline bool inside(const fs::path& p,const fs::path& parent){auto a=lower(full(p).wstring()),b=lower(full(parent).wstring());while(b.size()>3&&b.back()==L'\\')b.pop_back();b+=L'\\';return a.size()>b.size()&&a.compare(0,b.size(),b)==0;}
inline fs::path child(const fs::path& root,const std::wstring& relative){
    if(relative.empty()||relative.find(L':')!=std::wstring::npos)throw Fault("unsafe",utf8(relative));
    auto norm=relative;std::replace(norm.begin(),norm.end(),L'\\',L'/');if(norm[0]==L'/')throw Fault("unsafe");
    std::wstringstream parts(norm);std::wstring segment;
    while(std::getline(parts,segment,L'/')){
        if(segment.empty()||segment==L".")continue;
        if(segment==L".."||segment.back()==L'.'||segment.back()==L' ')throw Fault("unsafe",utf8(relative));
        for(auto c:segment)if(c<32||std::wstring(L"<>\"|?*").find(c)!=std::wstring::npos)throw Fault("unsafe",utf8(relative));
        auto stem=lower(segment.substr(0,segment.find(L'.')));
        if(stem==L"con"||stem==L"prn"||stem==L"aux"||stem==L"nul"||(stem.size()==4&&(stem.substr(0,3)==L"com"||stem.substr(0,3)==L"lpt")&&stem[3]>=L'0'&&stem[3]<=L'9'))throw Fault("unsafe",utf8(relative));
    }
    auto out=full(root/fs::path(relative));if(!inside(out,root))throw Fault("unsafe",utf8(relative));return out;
}
inline void noLinks(const fs::path& p){
    auto current=full(p);for(;;){DWORD flags=GetFileAttributesW(ioPath(current).c_str());if(flags==INVALID_FILE_ATTRIBUTES){auto error=GetLastError();if(error!=ERROR_FILE_NOT_FOUND&&error!=ERROR_PATH_NOT_FOUND)throw std::system_error(int(error),std::system_category());}else if(flags&FILE_ATTRIBUTE_REPARSE_POINT)throw Fault("unsafe",utf8(current.wstring()));auto up=current.parent_path();if(up==current||up.empty())break;current=up;}
}
inline std::vector<fs::path> files(const fs::path& root){
    noLinks(root);std::vector<fs::path> out;std::vector<fs::path> pending{root};
    while(!pending.empty()){auto dir=pending.back();pending.pop_back();for(auto& entry:fs::directory_iterator(dir)){noLinks(entry.path());if(entry.is_directory())pending.push_back(entry.path());else if(entry.is_regular_file())out.push_back(entry.path());else throw Fault("unsafe",utf8(entry.path().wstring()));}}
    return out;
}
inline uint64_t size(const fs::path& p){if(!fs::exists(p))return 0;if(fs::is_regular_file(p)){noLinks(p);return fs::file_size(p);}uint64_t total=0;for(auto& f:files(p)){auto n=fs::file_size(f);if(n>UINT64_MAX-total)throw Fault("space");total+=n;}return total;}
inline void space(const fs::path& p,uint64_t needed){auto dir=full(p);while(!fs::exists(dir)){auto up=dir.parent_path();if(up==dir)throw Fault("space");dir=up;}auto free=fs::space(dir).available;if(free<64ull*1024*1024||needed>free-64ull*1024*1024)throw Fault("space",std::to_string(needed/1048576)+" MiB + 64 MiB");}
inline void copyTree(const fs::path& source,const fs::path& target,const Progress& progress={}){
    if(lower(full(source).wstring())==lower(full(target).wstring())||inside(target,source))throw Fault("unsafe");
    noLinks(source);noLinks(target);fs::create_directories(target);std::vector<fs::path> pending{source};
    while(!pending.empty()){auto dir=pending.back();pending.pop_back();for(auto& entry:fs::directory_iterator(dir)){noLinks(entry.path());auto dst=child(target,entry.path().lexically_relative(source).wstring());noLinks(dst);if(entry.is_directory()){fs::create_directories(dst);pending.push_back(entry.path());}else if(entry.is_regular_file()){fs::create_directories(dst.parent_path());fs::copy_file(entry.path(),dst,fs::copy_options::overwrite_existing);if(progress)progress(entry.path().filename().wstring());}else throw Fault("unsafe");}}
}
inline void removeOwned(const fs::path& p,const fs::path& parent){if(!inside(p,parent))throw Fault("unsafe",utf8(p.wstring()));noLinks(p);if(fs::is_directory(p)){files(p);fs::remove_all(p);}else if(fs::exists(p))fs::remove(p);}
inline void atomicWrite(const fs::path& p,const std::string& text){noLinks(p);fs::create_directories(p.parent_path());auto tmp=p;tmp+=L"."+unique()+L".tmp";write(tmp,text);if(!MoveFileExW(ioPath(tmp).c_str(),ioPath(p).c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)){auto error=GetLastError();fs::remove(tmp);SetLastError(error);wincheck(false);}}
inline Json fingerprints(const fs::path& root){auto out=Json::array();auto all=files(root);std::sort(all.begin(),all.end());for(auto& f:all){auto entry=Json::object();entry["Path"]=utf8(f.lexically_relative(root).wstring());entry["Size"]=int64_t(fs::file_size(f));entry["Sha256"]=hash(f);out.items.push_back(entry);}return out;}
inline void verifyFiles(const fs::path& root,const Json& manifest,bool exact,const Progress& progress={}){
    if(manifest.kind!=Json::Array)throw Fault("checksum");auto all=files(root);if(exact&&all.size()!=manifest.items.size())throw Fault("checksum");std::set<std::wstring> seen;
    for(auto& entry:manifest.items){auto p=child(root,wide(entry.get("Path").str()));if(!seen.insert(lower(p.wstring())).second||!fs::is_regular_file(p)||entry.get("Size").kind!=Json::Number||entry.get("Size").integer()<0||fs::file_size(p)!=uint64_t(entry.get("Size").integer())||lower(wide(hash(p)))!=lower(wide(entry.get("Sha256").str())))throw Fault("checksum",utf8(p.wstring()));if(progress)progress(p.filename().wstring());}
}
}

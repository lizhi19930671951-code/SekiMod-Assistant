#pragma once
#include "archive.hpp"
#include "game.hpp"
#include <wininet.h>
namespace sma {
inline Json settings(){try{return readjson(data()/L"settings.json");}catch(...){return Json::object();}}
inline void saveSettings(const Json& j){atomicWrite(data()/L"settings.json",j.dump());}
inline std::vector<Json> catalog(const fs::path& app){
    auto root=readjson(app/L"assets"/L"catalog.json"),links=readjson(app/L"preset-downloads.json");if(root.kind!=Json::Array||links.kind!=Json::Array)throw Fault("catalog");
    for(auto& preset:root.items){for(auto& link:links.items)if(link.get("Id").str()==preset.get("Id").str()){preset["Url"]=link.get("Url");preset["FileName"]=link.get("FileName");}
        if(preset.get("Files").items.empty())try{auto learned=readjson(child(data()/L"manifests",wide(preset.get("Id").str())+L".json"));if(learned.get("Version").str()==preset.get("Version").str()&&learned.get("Url").str()==preset.get("Url").str())preset["Files"]=learned.get("Files");}catch(...){}
    }return root.items;
}
inline std::wstring presetName(const Json& p){return wide(p.get("Name").str()+" "+p.get("Version").str());}
inline void remember(const Json& preset){auto path=child(data()/L"manifests",wide(preset.get("Id").str())+L".json");atomicWrite(path,preset.dump());}
inline std::vector<fs::path> discover(const fs::path& saved){
    std::set<fs::path> found,libraries;auto add=[&](const fs::path& p){if(!p.empty()&&fs::is_regular_file(p/L"sekiro.exe"))found.insert(full(p));};
    auto steam=[&](const fs::path& p){if(p.empty())return;libraries.insert(p);try{auto vdf=read(p/L"steamapps"/L"libraryfolders.vdf");std::regex re("\"path\"\\s*\"([^\"]+)\"");for(auto it=std::sregex_iterator(vdf.begin(),vdf.end(),re);it!=std::sregex_iterator();++it){auto name=(*it)[1].str();size_t at=0;while((at=name.find("\\\\",at))!=std::string::npos)name.replace(at,2,"\\");libraries.insert(fs::path(wide(name)));}}catch(...){} };
    add(saved);steam(regstr(HKEY_CURRENT_USER,L"Software\\Valve\\Steam",L"SteamPath"));steam(known(FOLDERID_ProgramFilesX86)/L"Steam");
    DWORD drives=GetLogicalDrives();for(unsigned i=0;i<26;++i)if(drives&(1u<<i)){std::wstring drive{wchar_t(L'A'+i),L':',L'\\'};if(GetDriveTypeW(drive.c_str())!=DRIVE_FIXED)continue;for(auto n:{L"Steam",L"SteamLibrary",L"Games\\Steam",L"Program Files (x86)\\Steam"})steam(fs::path(drive)/n);for(auto n:{L"Games\\Sekiro",L"Games\\Sekiro Shadows Die Twice",L"Sekiro",L"Sekiro Shadows Die Twice"})add(fs::path(drive)/n);}
    for(auto& lib:libraries){add(lib/L"steamapps"/L"common"/L"Sekiro");try{auto text=read(lib/L"steamapps"/L"appmanifest_814380.acf");std::smatch match;if(std::regex_search(text,match,std::regex("\"installdir\"\\s*\"([^\"]+)\"")))add(child(lib/L"steamapps"/L"common",wide(match[1].str())));}catch(...){}}
    return {found.begin(),found.end()};
}
inline std::vector<fs::path> downloadFolders(){
    std::vector<fs::path> out;auto root=known(FOLDERID_LocalAppData)/L"Microsoft"/L"Edge"/L"User Data";
    try{for(auto& entry:fs::directory_iterator(root))if(entry.is_directory()&&(entry.path().filename()==L"Default"||entry.path().filename().wstring().find(L"Profile ")==0))try{auto j=readjson(entry.path()/L"Preferences");auto p=fs::path(wide(j.get("download").get("default_directory").str()));if(p.is_absolute())out.push_back(p);}catch(...){}}catch(...){}
    out.push_back(known(FOLDERID_Downloads));std::set<std::wstring> seen;out.erase(std::remove_if(out.begin(),out.end(),[&](const fs::path& p){return !seen.insert(lower(p.wstring())).second;}),out.end());return out;
}
inline std::wstring regexEscape(const std::wstring& s){std::wstring out;for(auto c:s){if(std::wstring(L".^$|()[]{}*+?\\").find(c)!=std::wstring::npos)out+=L'\\';out+=c;}return out;}
inline std::vector<fs::path> downloaded(const Json& preset,const Json& preferences,const std::vector<fs::path>& dirs){
    std::set<fs::path> found;auto saved=preferences.get("Downloads").get(preset.get("Id").str()).str();if(!saved.empty()){fs::path p(wide(saved));if(fs::exists(p)&&lower(p.extension().wstring())!=L".crdownload")found.insert(p);}
    fs::path expected(wide(preset.get("FileName").str()));auto pattern=std::wregex(L"^"+regexEscape(expected.stem().wstring())+L"(?: ?\\(\\d+\\))?"+regexEscape(expected.extension().wstring())+L"$",std::regex::icase);
    for(auto& dir:dirs)try{for(auto& entry:fs::directory_iterator(dir)){if(entry.is_regular_file()&&std::regex_match(entry.path().filename().wstring(),pattern))found.insert(entry.path());else if(entry.is_directory()&&entry.path().filename()==expected.stem())found.insert(entry.path());}}catch(...){}return {found.begin(),found.end()};
}
inline fs::path edge(){for(auto& base:{known(FOLDERID_ProgramFilesX86),known(FOLDERID_ProgramFiles),known(FOLDERID_LocalAppData)}){auto p=base/L"Microsoft"/L"Edge"/L"Application"/L"msedge.exe";if(fs::is_regular_file(p))return p;}return {};}
inline bool connected(){DWORD flags=0;return InternetGetConnectedState(&flags,0)!=FALSE;}
inline void validateUrl(const std::wstring& url){if(url.find(L"https://")!=0||url.find_first_of(L"\r\n\t\" ")!=std::wstring::npos||url.find(L'@')!=std::wstring::npos)throw Fault("linkMissing");}
inline void requireComplete(const fs::path& archive){auto tmp=archive;tmp+=L".crdownload";if(!isArchive(archive)||!fs::is_regular_file(archive)||!fs::file_size(archive)||fs::exists(tmp)||fs::exists(fs::path(archive.wstring()+L".partial")))throw Fault("archiveIncomplete");Handle lock(CreateFileW(ioPath(archive).c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr));if(lock.h==INVALID_HANDLE_VALUE)throw Fault("archiveIncomplete");}
struct ModStatus {bool installed=false;std::vector<std::wstring> complete,partial;};
struct Detection {
    struct Entry{uint64_t size;fs::file_time_type time;std::string hash;};std::map<fs::path,Entry> cache;
    std::string fingerprint(const fs::path& p){auto size=fs::file_size(p);auto time=fs::last_write_time(p);auto it=cache.find(p);if(it!=cache.end()&&it->second.size==size&&it->second.time==time)return it->second.hash;auto digest=hash(p);cache[p]={size,time,digest};return digest;}
    ModStatus scan(const fs::path& game,const std::vector<Json>& presets){ModStatus status;auto root=game/L"mods";if(!fs::is_directory(root))return status;auto all=files(root);status.installed=!all.empty();std::set<fs::path> current(all.begin(),all.end());for(auto it=cache.begin();it!=cache.end();)if(!current.count(it->first))it=cache.erase(it);else ++it;
        for(auto& p:presets){auto& manifest=p.get("Files");if(manifest.items.empty())continue;size_t paths=0,matching=0;for(auto& f:manifest.items){auto target=child(root,wide(f.get("Path").str()));if(!fs::is_regular_file(target))continue;++paths;if(fs::file_size(target)==uint64_t(f.get("Size").integer(-1))&&fingerprint(target)==f.get("Sha256").str())++matching;}if(matching==manifest.items.size())status.complete.push_back(presetName(p));else if(paths)status.partial.push_back(presetName(p)+L" ("+std::to_wstring(matching)+L"/"+std::to_wstring(manifest.items.size())+L")");}return status;
    }
};
}

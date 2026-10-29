#pragma once
#include "files.hpp"
namespace sma {
inline const std::vector<std::wstring> managed{L"dinput8.dll",L"modengine.ini",L"mods",L".sma-state.json"};
inline std::function<void(const std::string&)> testFault;
inline void validateGame(const fs::path& game){noLinks(game);if(!fs::is_regular_file(game/L"sekiro.exe"))throw Fault("noGame");for(auto& n:managed){auto p=child(game,n);noLinks(p);if(fs::exists(p)&&((n==L"mods"&&!fs::is_directory(p))||(n!=L"mods"&&!fs::is_regular_file(p))))throw Fault("invalidTarget");}}
inline void notRunning(){Handle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0));wincheck(snapshot.h!=INVALID_HANDLE_VALUE);PROCESSENTRY32W entry{};entry.dwSize=sizeof entry;if(Process32FirstW(snapshot.h,&entry))do{if(lower(entry.szExeFile)==L"sekiro.exe")throw Fault("running");}while(Process32NextW(snapshot.h,&entry));}
inline bool hasMods(const fs::path& game){return fs::is_directory(game/L"mods")&&!files(game/L"mods").empty();}
inline void ensureModsFolder(const fs::path& game){validateGame(game);noLinks(game/L"mods");if(!fs::exists(game/L"mods"))fs::create_directories(game/L"mods");}
inline Json state(const fs::path& game){if(fs::exists(game/L".sma-state.json"))return readjson(game/L".sma-state.json");return Json::object();}
inline fs::path backups(const fs::path& game){auto p=child(game,L".sma-backup");noLinks(p);return p;}
inline Json snapshotMeta(const fs::path& folder){
    noLinks(folder);auto meta=readjson(folder/L"snapshot.json");auto& present=meta.get("Exists");auto& list=meta.get("Files");
    if(!meta.get("Complete").boolean()||present.kind!=Json::Object||present.fields.size()!=managed.size()||list.kind!=Json::Array)throw Fault("backupInvalid");
    for(auto& n:managed){auto& item=present.get(utf8(n));if(item.kind!=Json::Boolean)throw Fault("backupInvalid");auto p=folder/n;if(item.flag&&((n==L"mods"&&!fs::is_directory(p))||(n!=L"mods"&&!fs::is_regular_file(p))))throw Fault("backupInvalid");}
    auto all=files(folder);all.erase(std::remove_if(all.begin(),all.end(),[&](const fs::path& p){return lower(p.lexically_relative(folder).wstring())==L"snapshot.json";}),all.end());
    if(all.size()!=list.items.size())throw Fault("backupInvalid");
    try{std::set<std::wstring> seen;for(auto& entry:list.items){auto relative=wide(entry.get("Path").str());auto p=child(folder,relative);auto first=relative.substr(0,relative.find_first_of(L"/\\"));if(std::find(managed.begin(),managed.end(),first)==managed.end()||!seen.insert(lower(p.wstring())).second)throw Fault("backupInvalid");if(!fs::is_regular_file(p)||entry.get("Size").integer(-1)<0||fs::file_size(p)!=uint64_t(entry.get("Size").integer())||hash(p)!=entry.get("Sha256").str())throw Fault("backupInvalid");}}
    catch(const Fault&){throw Fault("backupInvalid");}return meta;
}
inline void snapshotTo(const fs::path& game,const fs::path& target,const Progress& progress={}){
    fs::create_directories(target);auto meta=Json::object();meta["Exists"]=Json::object();
    for(auto& n:managed){auto p=game/n;meta["Exists"][utf8(n)]=fs::exists(p);if(fs::is_directory(p))copyTree(p,target/n,progress);else if(fs::exists(p))fs::copy_file(p,target/n,fs::copy_options::overwrite_existing);}
    meta["Files"]=fingerprints(target);meta["Complete"]=true;atomicWrite(target/L"snapshot.json",meta.dump());
}
inline void applySnapshot(const fs::path& source,const fs::path& game,const Progress& progress={}){
    auto meta=snapshotMeta(source);
    for(auto& n:managed){auto dst=game/n;removeOwned(dst,game);if(!meta.get("Exists").get(utf8(n)).boolean())continue;if(fs::is_directory(source/n))copyTree(source/n,dst,progress);else fs::copy_file(source/n,dst,fs::copy_options::overwrite_existing);}
}
inline void promote(const fs::path& backup){
    auto pending=backup/L"pending",latest=backup/L"latest",previous=backup/L"previous";
    if(!fs::exists(pending)){if(fs::exists(latest))snapshotMeta(latest);else throw Fault("backupInvalid");return;}
    snapshotMeta(pending);
    // Restartable even if the process stopped between either directory rename.
    if(fs::exists(latest)){removeOwned(previous,backup);fs::rename(latest,previous);}fs::rename(pending,latest);removeOwned(previous,backup);
}
inline bool needsRecovery(const fs::path& game){return fs::exists(backups(game)/L"journal.json");}
inline bool hasBackup(const fs::path& game){return fs::is_regular_file(backups(game)/L"latest"/L"snapshot.json");}
inline void recoverCore(const fs::path& game,const Progress& progress={}){
    auto b=backups(game),journal=b/L"journal.json";if(!fs::exists(journal))return;auto phase=read(journal);
    if(phase=="applying")applySnapshot(b/L"pending",game,progress);else if(phase=="committed")promote(b);else throw Fault("backupInvalid");fs::remove(journal);
}
struct GameLock {Handle handle;explicit GameLock(const fs::path& game){auto b=backups(game);fs::create_directories(b);handle.h=CreateFileW((b/L"lock").c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);wincheck(handle.h!=INVALID_HANDLE_VALUE);}};
inline void recover(const fs::path& game,const Progress& progress={}){validateGame(game);notRunning();GameLock guard(game);recoverCore(game,progress);}
inline void transaction(const fs::path& game,const std::function<void()>& mutate,const Progress& progress={}){
    validateGame(game);notRunning();GameLock guard(game);recoverCore(game,progress);auto b=backups(game),pending=b/L"pending",journal=b/L"journal.json";
    uint64_t bytes=0;for(auto& n:managed)bytes+=size(game/n);space(game,bytes);removeOwned(pending,b);snapshotTo(game,pending,progress);snapshotMeta(pending);atomicWrite(journal,"applying");
    try{notRunning();mutate();atomicWrite(journal,"committed");}catch(...){auto original=std::current_exception();applySnapshot(pending,game,progress);fs::remove(journal);removeOwned(pending,b);std::rethrow_exception(original);}promote(b);fs::remove(journal);
}
inline bool engineMatches(const fs::path& game,const fs::path& assets){for(auto& n:{L"dinput8.dll",L"modengine.ini"})if(!fs::is_regular_file(game/n)||hash(game/n)!=hash(assets/n))return false;return true;}
inline void install(const fs::path& game,const fs::path& assets,const fs::path& modRoot,bool merge,const std::string& name="",const std::string& version="",const Progress& progress={}){
    validateGame(game);notRunning();
    if(hash(assets/L"dinput8.dll")!="6EEEDBDAF656495922268D458E106ED9E646C350FBC57724A42CEBC0C3385505"||hash(assets/L"modengine.ini")!="226DD88049F65A7EC5A370F565FC87535ACBFCD3BC92178F619880EF7AE2E7DA")throw Fault("engineMissing");
    if(!modRoot.empty()&&files(modRoot).empty())throw Fault("empty");
    if(modRoot.empty()&&engineMatches(game,assets)&&fs::is_directory(game/L"mods")&&!state(game).get("PauseEngine").boolean()&&!needsRecovery(game))return;
    uint64_t bytes=modRoot.empty()?0:size(modRoot);for(auto& n:managed)bytes+=size(game/n);space(game,bytes);
    transaction(game,[&]{for(auto& n:{L"dinput8.dll",L"modengine.ini"})if(!fs::exists(game/n)||hash(game/n)!=hash(assets/n))fs::copy_file(assets/n,game/n,fs::copy_options::overwrite_existing);if(testFault)testFault("engine");
        if(!modRoot.empty()){if(!merge)removeOwned(game/L"mods",game);copyTree(modRoot,game/L"mods",progress);}else fs::create_directories(game/L"mods");if(testFault)testFault("mods");
        auto st=state(game);st["PauseEngine"]=false;if(!modRoot.empty()){st["Name"]=name;st["Version"]=version;}atomicWrite(game/L".sma-state.json",st.dump());},progress);
}
inline void clearMods(const fs::path& game,const Progress& progress={}){transaction(game,[&]{removeOwned(game/L"mods",game);fs::create_directories(game/L"mods");if(testFault)testFault("clear");auto st=state(game);st["Name"]="";st["Version"]="";atomicWrite(game/L".sma-state.json",st.dump());},progress);}
inline void restore(const fs::path& game,const Progress& progress={}){transaction(game,[&]{applySnapshot(backups(game)/L"latest",game,progress);auto st=state(game);st["PauseEngine"]=true;atomicWrite(game/L".sma-state.json",st.dump());},progress);}
}

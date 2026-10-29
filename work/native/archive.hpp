#pragma once
#include "files.hpp"
#include <future>
namespace sma {
struct ProcessResult{DWORD code=1;std::string out,error;};
inline ProcessResult run(const fs::path& exe,const std::vector<std::wstring>& args,const fs::path& output={},uint64_t budget=128ull*1024*1024){
    SECURITY_ATTRIBUTES sa{sizeof sa,nullptr,TRUE};Handle outRead,outWrite,errRead,errWrite;wincheck(CreatePipe(&outRead.h,&outWrite.h,&sa,0));wincheck(CreatePipe(&errRead.h,&errWrite.h,&sa,0));wincheck(SetHandleInformation(outRead.h,HANDLE_FLAG_INHERIT,0));wincheck(SetHandleInformation(errRead.h,HANDLE_FLAG_INHERIT,0));
    Handle nullInput(CreateFileW(L"NUL",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,&sa,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr));wincheck(nullInput.h!=INVALID_HANDLE_VALUE);
    std::wstring command=quote(exe.wstring());for(auto& arg:args)command+=L" "+quote(arg);
    STARTUPINFOW start{};start.cb=sizeof start;start.dwFlags=STARTF_USESTDHANDLES;start.hStdOutput=outWrite.h;start.hStdError=errWrite.h;start.hStdInput=nullInput.h;PROCESS_INFORMATION pi{};
    // Only explicit pipe handles are inheritable; no shell interpretation is used.
    wincheck(CreateProcessW(exe.c_str(),command.data(),nullptr,nullptr,TRUE,CREATE_NO_WINDOW|CREATE_SUSPENDED,nullptr,nullptr,&start,&pi));Handle process(pi.hProcess),thread(pi.hThread),job(CreateJobObjectW(nullptr,nullptr));
    if(!job.h){TerminateProcess(process.h,1);wincheck(false);}JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if(!SetInformationJobObject(job.h,JobObjectExtendedLimitInformation,&limits,sizeof limits)||!AssignProcessToJobObject(job.h,process.h)){TerminateProcess(process.h,1);wincheck(false);}ResumeThread(thread.h);
    CloseHandle(outWrite.h);outWrite.h=INVALID_HANDLE_VALUE;CloseHandle(errWrite.h);errWrite.h=INVALID_HANDLE_VALUE;
    ProcessResult result;std::exception_ptr streamError;
    auto drain=std::async(std::launch::async,[&]{char b[16384];DWORD n=0;while(ReadFile(errRead.h,b,sizeof b,&n,nullptr)&&n){if(result.error.size()<1024*1024)result.error.append(b,std::min<size_t>(n,1024*1024-result.error.size()));}});
    auto stream=std::async(std::launch::async,[&]{try{std::ofstream file;if(!output.empty()){file.open(ioPath(output),std::ios::binary|std::ios::trunc);if(!file)throw Fault("write");}char b[131072];DWORD n=0;uint64_t count=0;while(ReadFile(outRead.h,b,sizeof b,&n,nullptr)&&n){count+=n;if(count>budget)throw Fault("sizeLimit");if(output.empty())result.out.append(b,n);else{file.write(b,n);if(!file)throw Fault("write");}}}catch(...){streamError=std::current_exception();TerminateJobObject(job.h,1);}});
    if(WaitForSingleObject(process.h,5*60*1000)!=WAIT_OBJECT_0){TerminateJobObject(job.h,1);WaitForSingleObject(process.h,10000);stream.wait();drain.wait();throw Fault("archiveTimeout");}
    stream.get();drain.get();if(streamError)std::rethrow_exception(streamError);GetExitCodeProcess(process.h,&result.code);return result;
}
inline bool isArchive(const fs::path& p){auto name=lower(p.filename().wstring());for(auto& ext:{L".zip",L".rar",L".7z",L".tar",L".gz",L".bz2",L".xz",L".tgz",L".tbz2",L".txz",L".zst",L".cab",L".iso",L".001"})if(name.size()>=wcslen(ext)&&name.substr(name.size()-wcslen(ext))==ext)return true;return false;}
inline void extract(const fs::path& seven,const fs::path& archive,const fs::path& target,const std::wstring& password=L"",const Progress& progress={}){
    noLinks(archive);noLinks(target);fs::create_directories(target);
    Handle archiveLock(CreateFileW(ioPath(archive).c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr));wincheck(archiveLock.h!=INVALID_HANDLE_VALUE);
    std::wstring pass=L"-p"+(password.empty()?unique():password);auto listing=run(seven,{L"l",L"-slt",L"-ba",L"-sccUTF-8",L"-y",pass,archive.wstring()});if(listing.code)throw Fault("archive",listing.error);
    listing.out.erase(std::remove(listing.out.begin(),listing.out.end(),'\r'),listing.out.end());
    std::vector<std::map<std::string,std::string>> records;std::map<std::string,std::string> record;std::istringstream input(listing.out);std::string line;
    while(std::getline(input,line)){if(line.empty()){if(!record.empty()){records.push_back(record);record.clear();}continue;}auto at=line.find(" = ");if(at!=std::string::npos){auto key=line.substr(0,at);if(record.count(key))throw Fault("unsafe");record[key]=line.substr(at+3);}}
    if(!record.empty())records.push_back(record);if(records.size()>50000)throw Fault("sizeLimit");
    std::set<std::wstring> seen;uint64_t written=0;auto available=fs::space(target).available;if(available<128ull*1024*1024)throw Fault("space");uint64_t budget=std::min<uint64_t>(100ull*1024*1024*1024,available-128ull*1024*1024);
    for(auto& item:records){
        auto name=item.count("Path")?wide(item["Path"]):archive.stem().wstring();if(name==L"."||name==L"./")continue;
        while(!name.empty()&&(name.back()==L'/'||name.back()==L'\\'))name.pop_back();auto dst=child(target,name);if(!seen.insert(lower(dst.wstring())).second)throw Fault("unsafe");
        auto attributes=item["Attributes"]+" "+item["Mode"];
        for(auto& kv:item)if(kv.first.find("Link")!=std::string::npos&&!kv.second.empty())throw Fault("unsafe");
        if(attributes.find("lrwx")!=std::string::npos||attributes.find("prw")!=std::string::npos||attributes.find("brw")!=std::string::npos||attributes.find("crw")!=std::string::npos)throw Fault("unsafe");
        if(item["Folder"]=="+"||(!attributes.empty()&&attributes[0]=='D')||attributes.find("drwx")!=std::string::npos)continue;
        bool known=!item["Size"].empty();uint64_t expected=budget-written;if(known){try{size_t used=0;expected=std::stoull(item["Size"],&used);if(used!=item["Size"].size()||item["Size"][0]=='-')throw Fault("unsafe");}catch(...){throw Fault("unsafe");}}
        if(expected>budget-written)throw Fault("space");fs::create_directories(dst.parent_path());std::vector<std::wstring> args{L"x",L"-so",L"-spd",L"-y",L"-sccUTF-8",pass,archive.wstring()};if(item.count("Path")){args.push_back(L"--");args.push_back(name);}
        auto result=run(seven,args,dst,expected);if(result.code||(known&&fs::file_size(dst)!=expected))throw Fault("archive",result.error);written+=fs::file_size(dst);if(progress)progress(name);
    }if(files(target).empty())throw Fault("empty");
}
inline const std::set<std::wstring> standard{L"action",L"chr",L"param",L"parts",L"map",L"sound",L"event",L"menu",L"msg",L"mtd",L"obj",L"script",L"sfx",L"other",L"cutscene"};
inline std::vector<fs::path> modRoots(const fs::path& root){files(root);std::vector<fs::path> pending{root},found;for(size_t i=0;i<pending.size();++i){auto dir=pending[i];bool recognized=false;std::vector<fs::path> sub;for(auto& entry:fs::directory_iterator(dir))if(entry.is_directory()){sub.push_back(entry.path());if(standard.count(lower(entry.path().filename().wstring())))recognized=true;}if(recognized)found.push_back(dir);else pending.insert(pending.end(),sub.begin(),sub.end());}return found;}
inline bool blocked(const fs::path& p){static const std::set<std::wstring> ext{L".exe",L".dll",L".com",L".bat",L".cmd",L".ps1",L".msi",L".scr",L".vbs",L".js",L".lnk",L".url"};return ext.count(lower(p.extension().wstring()))||lower(p.filename().wstring())==L"modengine.ini";}
struct Staging {
    fs::path folder,merged;std::vector<std::wstring> skipped,conflicts;
    Staging(){folder=data()/L"staging"/unique();merged=folder/L"merged";noLinks(folder);fs::create_directories(merged);}
    ~Staging(){try{removeOwned(folder,data()/L"staging");}catch(...){}}
    void merge(const fs::path& root,const Progress& progress={}){for(auto& f:files(root)){auto relative=f.lexically_relative(root);if(blocked(f)){skipped.push_back(relative.wstring());continue;}auto dst=child(merged,relative.wstring());if(fs::is_directory(dst))throw Fault("conflict");if(fs::exists(dst))conflicts.push_back(relative.wstring());fs::create_directories(dst.parent_path());fs::copy_file(f,dst,fs::copy_options::overwrite_existing);if(progress)progress(relative.wstring());}}
};
}


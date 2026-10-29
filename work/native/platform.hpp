#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <bcrypt.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <set>
#include <thread>
#include <chrono>
#include <atomic>
#include <cwctype>
#include <exception>
#include <regex>
#include "json.hpp"
namespace sma {
static_assert(sizeof(void*)==8,"Build the Windows x64 target; use x86_64-w64-mingw32-clang++.");
namespace fs=std::filesystem;
inline fs::path ioPath(const fs::path& p){auto value=fs::absolute(p).lexically_normal().wstring();if(value.find(L"\\\\?\\")==0)return value;if(value.find(L"\\\\")==0)return fs::path(L"\\\\?\\UNC\\"+value.substr(2));return fs::path(L"\\\\?\\"+value);}
using Progress=std::function<void(const std::wstring&)>;
struct Fault:std::runtime_error{std::string key;Fault(std::string k,std::string detail=""):std::runtime_error(detail.empty()?k:detail),key(std::move(k)){} };
inline std::wstring wide(const std::string& s){if(s.empty())return {};int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s.data(),int(s.size()),nullptr,0);if(!n)throw Fault("invalidUtf8");std::wstring out(n,0);MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s.data(),int(s.size()),out.data(),n);return out;}
inline std::string utf8(const std::wstring& s){if(s.empty())return {};int n=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,s.data(),int(s.size()),nullptr,0,nullptr,nullptr);if(!n)throw Fault("invalidUtf8");std::string out(n,0);WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,s.data(),int(s.size()),out.data(),n,nullptr,nullptr);return out;}
inline std::wstring lower(std::wstring s){std::transform(s.begin(),s.end(),s.begin(),[](wchar_t c){return wchar_t(towlower(c));});return s;}
struct Handle{HANDLE h=INVALID_HANDLE_VALUE;Handle()=default;explicit Handle(HANDLE p):h(p){}~Handle(){if(h&&h!=INVALID_HANDLE_VALUE)CloseHandle(h);}Handle(const Handle&)=delete;Handle& operator=(const Handle&)=delete;};
inline void wincheck(bool good){if(!good)throw std::system_error(int(GetLastError()),std::system_category());}
inline fs::path executable(){std::wstring b(32768,0);DWORD n=GetModuleFileNameW(nullptr,b.data(),DWORD(b.size()));wincheck(n&&n<b.size());b.resize(n);return b;}
inline fs::path known(REFKNOWNFOLDERID id){PWSTR p=nullptr;HRESULT hr=SHGetKnownFolderPath(id,0,nullptr,&p);if(FAILED(hr))throw Fault("folder");fs::path result(p);CoTaskMemFree(p);return result;}
inline fs::path data(){wchar_t p[32768];DWORD n=GetEnvironmentVariableW(L"SMA_DATA",p,32768);return n&&n<32768?fs::path(p):known(FOLDERID_LocalAppData)/L"SekiroModAssistant";}
inline std::string read(const fs::path& p,size_t max=32*1024*1024){std::ifstream in(ioPath(p),std::ios::binary);if(!in)throw Fault("read",utf8(p.wstring()));std::string out;char b[65536];while(in){in.read(b,sizeof b);out.append(b,size_t(in.gcount()));if(out.size()>max)throw Fault("sizeLimit");}if(in.bad())throw Fault("read");return out;}
inline void write(const fs::path& p,const std::string& s){std::ofstream out(ioPath(p),std::ios::binary|std::ios::trunc);if(!out)throw Fault("write",utf8(p.wstring()));out.write(s.data(),std::streamsize(s.size()));out.flush();if(!out)throw Fault("write",utf8(p.wstring()));}
inline Json readjson(const fs::path& p){return Json::parse(read(p));}
inline std::wstring unique(){GUID g{};if(FAILED(CoCreateGuid(&g)))throw Fault("guid");wchar_t b[40];StringFromGUID2(g,b,40);return b;}
inline std::string hash(const fs::path& p){
    BCRYPT_ALG_HANDLE alg=nullptr;BCRYPT_HASH_HANDLE h=nullptr;
    if(BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA256_ALGORITHM,nullptr,0)<0)throw Fault("hash");
    struct Clean{BCRYPT_ALG_HANDLE& a;BCRYPT_HASH_HANDLE& h;~Clean(){if(h)BCryptDestroyHash(h);if(a)BCryptCloseAlgorithmProvider(a,0);}}clean{alg,h};
    DWORD len=0,actual=0;if(BCryptGetProperty(alg,BCRYPT_OBJECT_LENGTH,reinterpret_cast<PUCHAR>(&len),sizeof len,&actual,0)<0)throw Fault("hash");std::vector<unsigned char> object(len);
    if(BCryptCreateHash(alg,&h,object.data(),len,nullptr,0,0)<0)throw Fault("hash");
    std::ifstream in(ioPath(p),std::ios::binary);if(!in)throw Fault("read",utf8(p.wstring()));unsigned char b[131072];while(in){in.read(reinterpret_cast<char*>(b),sizeof b);auto n=in.gcount();if(n&&BCryptHashData(h,b,ULONG(n),0)<0)throw Fault("hash");}if(in.bad())throw Fault("read");unsigned char digest[32];if(BCryptFinishHash(h,digest,32,0)<0)throw Fault("hash");std::string out;const char* digits="0123456789ABCDEF";for(auto c:digest){out+=digits[c>>4];out+=digits[c&15];}return out;
}
inline std::wstring quote(const std::wstring& s){std::wstring out=L"\"";size_t slashes=0;for(auto c:s){if(c==L'\\'){++slashes;continue;}if(c==L'"')out.append(slashes*2+1,L'\\');else out.append(slashes,L'\\');out+=c;slashes=0;}out.append(slashes*2,L'\\');return out+L'"';}
inline void shell(const fs::path& exe,const std::wstring& args=L"",const wchar_t* verb=L"open"){
    SHELLEXECUTEINFOW s{};s.cbSize=sizeof s;s.fMask=SEE_MASK_NOCLOSEPROCESS;s.lpVerb=verb;s.lpFile=exe.c_str();s.lpParameters=args.c_str();s.nShow=SW_SHOWNORMAL;wincheck(ShellExecuteExW(&s)!=FALSE);if(s.hProcess)CloseHandle(s.hProcess);
}
inline std::wstring regstr(HKEY hive,const wchar_t* key,const wchar_t* value){wchar_t b[32768];DWORD n=sizeof b;if(RegGetValueW(hive,key,value,RRF_RT_REG_SZ,nullptr,b,&n)!=ERROR_SUCCESS)return {};return b;}
}

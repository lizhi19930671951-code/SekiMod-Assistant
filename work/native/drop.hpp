#pragma once
#include "platform.hpp"
#include <commctrl.h>
namespace sma {
// Use the shell's HDROP protocol, including its cross-integrity memory transfer.
// Permit only the drop messages on the specific receiving HWNDs, never globally.
inline void acceptShellDrops(HWND window){
    for(UINT message:{UINT(WM_DROPFILES),UINT(WM_COPYDATA),UINT(0x0049)})
        wincheck(ChangeWindowMessageFilterEx(window,message,MSGFLT_ALLOW,nullptr)!=FALSE);
    DragAcceptFiles(window,TRUE);
}
inline LRESULT CALLBACK listDropProc(HWND window,UINT message,WPARAM a,LPARAM b,UINT_PTR,DWORD_PTR){
    if(message==WM_DROPFILES)return SendMessageW(GetAncestor(window,GA_ROOT),message,a,b);
    if(message==WM_NCDESTROY){DragAcceptFiles(window,FALSE);RemoveWindowSubclass(window,listDropProc,3);}
    return DefSubclassProc(window,message,a,b);
}
inline void acceptListDrops(HWND window){
    wincheck(SetWindowSubclass(window,listDropProc,3,0)!=FALSE);
    acceptShellDrops(window);
}
inline std::vector<fs::path> takeShellDrop(HDROP drop){
    struct Finish {HDROP value;~Finish(){DragFinish(value);}} finish{drop};
    std::vector<fs::path> paths;
    UINT count=DragQueryFileW(drop,0xffffffff,nullptr,0);
    for(UINT i=0;i<count;++i){
        UINT length=DragQueryFileW(drop,i,nullptr,0);
        std::wstring path(size_t(length)+1,0);
        if(!length||DragQueryFileW(drop,i,path.data(),length+1)!=length)continue;
        path.resize(length);paths.emplace_back(path);
    }
    return paths;
}
}

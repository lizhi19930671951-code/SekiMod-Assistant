#pragma once
namespace sma {
inline constexpr UINT_PTR hoverTimer=0x534d;
struct HoverFade {
 double from=0,to=0;ULONGLONG start=0;
 double value(ULONGLONG now=GetTickCount64()) const {double t=std::min(1.0,double(now-start)/120.0);return from+(to-from)*t;}
 bool running() const{return GetTickCount64()-start<120&&from!=to;}
 void target(bool on){double next=on?1:0;if(next==to)return;from=value();to=next;start=GetTickCount64();}
};
inline std::map<HWND,HoverFade> controlFades;
inline std::map<HWND,std::map<int,HoverFade>> captionFades;
inline double controlGlow(HWND w){auto it=controlFades.find(w);return it==controlFades.end()?0:it->second.value();}
inline void refreshControl(HWND w){RedrawWindow(w,nullptr,nullptr,RDW_INVALIDATE|RDW_FRAME);}
inline void controlHover(HWND w,bool on){auto& fade=controlFades[w];on=on&&IsWindowEnabled(w);if(fade.to==(on?1:0))return;fade.target(on);SetTimer(w,hoverTimer,15,nullptr);refreshControl(w);}
inline void fieldBorder(HWND w,HDC dc,bool nonclient){wchar_t cls[32]{};GetClassNameW(w,cls,32);if(lower(std::wstring(cls))==L"button")return;double glow=IsWindowEnabled(w)?std::max(controlGlow(w),GetFocus()==w?1.0:0.0):0;if(glow<=0)return;RECT r{};if(nonclient){GetWindowRect(w,&r);OffsetRect(&r,-r.left,-r.top);}else GetClientRect(w,&r);auto brush=CreateSolidBrush(RGB(int(67+65*glow),int(91+110*glow),int(116+126*glow)));FrameRect(dc,&r,brush);DeleteObject(brush);}
inline LRESULT CALLBACK hoverProc(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR,DWORD_PTR){
  if(GetDlgCtrlID(w)==109&&(m==WM_MOUSEMOVE||m==WM_MOUSELEAVE)){auto owner=GetAncestor(w,GA_ROOT);auto tip=static_cast<HWND>(GetPropW(owner,L"SMA.Tooltip"));TOOLINFOW info{};info.cbSize=sizeof info;info.hwnd=owner;info.uId=1;auto text=tr("importDestination");info.lpszText=text.data();if(m==WM_MOUSEMOVE){SendMessageW(tip,TTM_UPDATETIPTEXTW,0,reinterpret_cast<LPARAM>(&info));POINT p{};GetCursorPos(&p);SendMessageW(tip,TTM_TRACKPOSITION,0,MAKELPARAM(p.x+12,p.y+24));}SendMessageW(tip,TTM_TRACKACTIVATE,m==WM_MOUSEMOVE,reinterpret_cast<LPARAM>(&info));}
  if(m==WM_MOUSEMOVE){TRACKMOUSEEVENT track{sizeof track,TME_LEAVE,w,0};TrackMouseEvent(&track);controlHover(w,true);}
 if(m==WM_MOUSELEAVE||(m==WM_ENABLE&&a==FALSE))controlHover(w,false);
 if(m==WM_SETFOCUS||m==WM_KILLFOCUS)refreshControl(w);
 if(m==WM_TIMER&&a==hoverTimer){auto it=controlFades.find(w);if(it==controlFades.end()||!it->second.running())KillTimer(w,hoverTimer);refreshControl(w);return 0;}
 if(m==WM_PAINT||m==WM_PRINTCLIENT){auto result=DefSubclassProc(w,m,a,b);auto dc=m==WM_PRINTCLIENT?reinterpret_cast<HDC>(a):GetDC(w);fieldBorder(w,dc,false);if(m!=WM_PRINTCLIENT)ReleaseDC(w,dc);return result;}
 if(m==WM_NCPAINT){auto result=DefSubclassProc(w,m,a,b);auto dc=GetWindowDC(w);fieldBorder(w,dc,true);ReleaseDC(w,dc);return result;}
 if(m==WM_NCDESTROY){KillTimer(w,hoverTimer);controlFades.erase(w);RemoveWindowSubclass(w,hoverProc,2);}
 return DefSubclassProc(w,m,a,b);
}
}

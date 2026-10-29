// Isolated Win32 regression harness: no settings, game or install operations.
#include "skin.hpp"
using namespace sma;
LRESULT CALLBACK probeProc(HWND w,UINT m,WPARAM a,LPARAM b){LRESULT result=0;if(chromeMessage(w,m,a,b,result))return result;return DefWindowProcW(w,m,a,b);}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,LPWSTR,int){
 CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);initSkin();WNDCLASSW cls{};cls.hInstance=instance;cls.lpszClassName=L"CaptionRegression";cls.lpfnWndProc=probeProc;RegisterClassW(&cls);
 HWND w=CreateWindowW(cls.lpszClassName,L"Caption regression",WS_POPUP|WS_THICKFRAME,0,0,800,600,nullptr,nullptr,instance,nullptr);chromeAttach(w,true);ShowWindow(w,SW_SHOWNOACTIVATE);UpdateWindow(w);ValidateRect(w,nullptr);std::string report;bool ok=true;RECT region{};
 for(int hit:{HTCAPTION,HTLEFT,HTTOP,HTCLIENT}){SendMessageW(w,WM_NCMOUSEMOVE,hit,0);bool invalid=GetUpdateRect(w,&region,FALSE);ok=ok&&!invalid;report+=(invalid?"FAIL ":"PASS ")+std::string("unrelated mouse move leaves caption images untouched\n");ValidateRect(w,nullptr);}
 chromeHover(w,0);ValidateRect(w,nullptr);chromeHover(w,HTMINBUTTON);GetUpdateRect(w,&region,FALSE);auto expected=captionRect(w,HTMINBUTTON);bool local=EqualRect(&region,&expected);ok=ok&&local;report+=(local?"PASS ":"FAIL ")+std::string("hover redraw limited to affected image\n");
 write(executable().parent_path()/L"caption-regression.txt",report);DestroyWindow(w);freeSkin();CoUninitialize();return ok?0:1;
}

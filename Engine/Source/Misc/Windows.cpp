/******************************************************************************

   On Linux, windows are organized into up to 3 HWND families:
   1. resizable extent (or none if window is not resizable)
   2. borders
   3. client
 
/******************************************************************************/
#include "stdafx.h"
#if MAC
   #include "../Platforms/Mac/MyWindow.h"
   #include "../Platforms/Mac/MyOpenGLView.h"
#endif
namespace EE{
/******************************************************************************/
#if WINDOWS_OLD

#include <windowsx.h>
#include <Dbt.h>
#define GET_POINTERID_WPARAM LOWORD

#define WM_POINTERDOWN           0x0246
#define WM_POINTERUP             0x0247
#define WM_POINTERENTER          0x0249
#define WM_POINTERLEAVE          0x024A
#define WM_POINTERUPDATE         0x0245
#define WM_POINTERCAPTURECHANGED 0x024C
#define WM_DPICHANGED            0x02E0

typedef enum tagPOINTER_INPUT_TYPE
{
   PT_POINTER=0x00000001,
   PT_TOUCH  =0x00000002,
   PT_PEN    =0x00000003,
   PT_MOUSE  =0x00000004,
}POINTER_INPUT_TYPE;

static BOOL (WINAPI *GetPointerType)(UINT32 pointerId, POINTER_INPUT_TYPE *pointerType);

static HPOWERNOTIFY PowerNotify;

struct WindowCaptureEx
{
   HDC     dc;
   HBITMAP bitmap;
   VecI2   size;

   WindowCaptureEx() {dc=null; bitmap=null; size.zero();}
  ~WindowCaptureEx() {del();}

   void del()
   {
      if(bitmap){DeleteObject(bitmap); bitmap=null;}
      if(dc    ){DeleteDC    (dc    ); dc    =null;}
      size.zero();
   }

   Bool capture(Image &image, Ptr hwnd=null)
   {
      Bool ok=false;
   #if DX11
      if(hwnd==App.hwnd() && SwapChainDesc.SwapEffect!=DXGI_SWAP_EFFECT_DISCARD) // on DX10+ when swap chain flip mode is enabled, using 'GetDC' will result in a black image
      {
         Renderer.capture(image, -1, -1, -1, -1, 1, false);
      }else
   #endif
      if(HDC src_dc=GetDC((HWND)hwnd))
      {
       //if(HBITMAP src_bitmap=(HBITMAP)GetCurrentObject(src_dc, OBJ_BITMAP))
         {
          //BITMAP bmp; if(GetObject(src_bitmap, SIZE(bmp), &bmp))
            {
             //Int width=bmp.bmWidth, height=bmp.bmHeight; don't use bitmap size because it returns full window size (including borders, not client only)
               Int width, height;
               if(hwnd)
               {
                  VecI2 size=WindowSize(true, hwnd);
                  width =size.x;
                  height=size.y;
               }else
               {
                  width =D.screenW();
                  height=D.screenH();
               }

               if(width!=size.x || height!=size.y)
               {
                  if(!dc)dc=CreateCompatibleDC(src_dc);
                  if(bitmap){DeleteObject(bitmap); bitmap=null;}
                  if(bitmap=CreateCompatibleBitmap(src_dc, width, height)){size.set(width, height); SelectObject(dc, bitmap);}else size.zero();
               }
               
               if(bitmap && BitBlt(dc, 0, 0, size.x, size.y, src_dc, 0, 0, SRCCOPY))
               {
                  if(image.hwType()!=IMAGE_B8G8R8A8 || image.size()!=size || image.d()!=1 || image.pitch()!=image.w()*image.bytePP())image.createSoftTry(size.x, size.y, 1, IMAGE_B8G8R8A8);
                  if(image.is() && image.lock(LOCK_WRITE))
                  {
                     BITMAPINFO bi; Zero(bi);
                     bi.bmiHeader.biSize  =SIZE(bi.bmiHeader); 
                     bi.bmiHeader.biWidth = image.w();
                     bi.bmiHeader.biHeight=-image.h(); // positive height would flip the image
                     bi.bmiHeader.biPlanes=1;
                     bi.bmiHeader.biBitCount=32;
                     bi.bmiHeader.biCompression=BI_RGB;
                     ok=(GetDIBits(dc, bitmap, 0, image.h(), image.data(), &bi, DIB_RGB_COLORS)>0);
                     image.unlock();
                  }
               }
            }
         }
         ReleaseDC((HWND)hwnd, src_dc);
      }
      return ok;
   }
};
void WindowCapture::del()
{
   Delete((WindowCaptureEx*&)data);
}
Bool WindowCapture::capture(Image &image, Ptr hwnd)
{
   WindowCaptureEx* &wc=(WindowCaptureEx*&)data;
   if(!wc)New(wc);
   return wc->capture(image, hwnd);
}
#else
void WindowCapture::del()
{
}
Bool WindowCapture::capture(Image &image, Ptr hwnd)
{
   return false;
}
#endif
/******************************************************************************/
#if WINDOWS_OLD
static ITaskbarList3 *TaskbarList;

void WindowSetText(C Str &text, Ptr hwnd)
{
   SetWindowText((HWND)hwnd, text);
}
Str WindowGetText(Ptr hwnd)
{
   wchar_t temp[16*1024]; temp[0]='\0'; GetWindowText((HWND)hwnd, temp, Elms(temp)); return temp;
}
void WindowMinimize(Bool force, Ptr hwnd)
{
   if(force)ShowWindow((HWND)hwnd, SW_MINIMIZE);
   else    PostMessage((HWND)hwnd, WM_SYSCOMMAND, SC_MINIMIZE, NULL);
}
void WindowMaximize(Bool force, Ptr hwnd)
{
   if(force)ShowWindow((HWND)hwnd, SW_MAXIMIZE);
   else    PostMessage((HWND)hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, NULL);
}
void WindowReset(Bool force, Ptr hwnd)
{
   if(force)ShowWindow((HWND)hwnd, SW_RESTORE);
   else    PostMessage((HWND)hwnd, WM_SYSCOMMAND, SC_RESTORE, NULL);
}
void WindowToggle(Bool force, Ptr hwnd)
{
   if(WindowMaximized(hwnd))WindowReset   (force, hwnd);
   else                     WindowMaximize(force, hwnd);
}
void WindowActivate(Ptr hwnd)
{
   if(WindowMinimized(hwnd))WindowReset(false, hwnd);
   UIntPtr act_thread=GetThreadIdFromWindow(WindowActive()),
           cur_thread=GetThreadId          (              );
   if(cur_thread!=act_thread)AttachThreadInput(act_thread, cur_thread, true);
   BringWindowToTop   ((HWND)hwnd);
   SetForegroundWindow((HWND)hwnd);
   if(cur_thread!=act_thread)AttachThreadInput(act_thread, cur_thread, false);
}
void WindowHide(Ptr hwnd)
{
   ShowWindow((HWND)hwnd, SW_HIDE);
}
void WindowShow(Ptr hwnd)
{
   ShowWindow((HWND)hwnd, SW_SHOWNA);
}
void WindowClose(Ptr hwnd)
{
   PostMessage((HWND)hwnd, WM_SYSCOMMAND, SC_CLOSE, NULL);
}
void WindowFlash(Ptr hwnd)
{
   FlashWindow((HWND)hwnd, true);
}
/******************************************************************************/
void WindowSetNormal  (              Ptr hwnd) {if(TaskbarList) TaskbarList->SetProgressState((HWND)hwnd, TBPF_NOPROGRESS   );}
void WindowSetWorking (              Ptr hwnd) {if(TaskbarList) TaskbarList->SetProgressState((HWND)hwnd, TBPF_INDETERMINATE);}
void WindowSetProgress(Flt progress, Ptr hwnd) {if(TaskbarList){TaskbarList->SetProgressState((HWND)hwnd, TBPF_NORMAL       ); TaskbarList->SetProgressValue((HWND)hwnd, RoundU(Sat(progress)*65536), 65536);}}
void WindowSetPaused  (Flt progress, Ptr hwnd) {if(TaskbarList){TaskbarList->SetProgressState((HWND)hwnd, TBPF_PAUSED       ); TaskbarList->SetProgressValue((HWND)hwnd, RoundU(Sat(progress)*65536), 65536);}}
void WindowSetError   (Flt progress, Ptr hwnd) {if(TaskbarList){TaskbarList->SetProgressState((HWND)hwnd, TBPF_ERROR        ); TaskbarList->SetProgressValue((HWND)hwnd, RoundU(Sat(progress)*65536), 65536);}}
/******************************************************************************/
Byte WindowGetAlpha(Ptr hwnd)
{
   BYTE alpha; if(hwnd && GetLayeredWindowAttributes((HWND)hwnd, null, &alpha, null))return alpha;
   return 255;
}
void WindowAlpha(Byte alpha, Ptr hwnd)
{
   if(hwnd)
   {
      if(alpha==255)
      {
         SetWindowLong((HWND)hwnd, GWL_EXSTYLE, GetWindowLong((HWND)hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
      }else
      {
         SetWindowLong((HWND)hwnd, GWL_EXSTYLE, GetWindowLong((HWND)hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
         SetLayeredWindowAttributes((HWND)hwnd, 0, alpha, LWA_ALPHA);
      }
   }
}
/******************************************************************************/
void WindowMove(Int dx, Int dy, Ptr hwnd)
{
   if(dx || dy)
   {
      RECT rect; GetWindowRect((HWND)hwnd, &rect);
      MoveWindow((HWND)hwnd, rect.left+dx, rect.top+dy, rect.right-rect.left, rect.bottom-rect.top, true);
   }
}
void WindowPos(Int x, Int y, Ptr hwnd)
{
   RECT rect; GetWindowRect((HWND)hwnd, &rect);
   MoveWindow((HWND)hwnd, x, y, rect.right-rect.left, rect.bottom-rect.top, true);
}
void WindowSize(Int w, Int  h, Bool client, Ptr hwnd)
{
   RECT rect; GetWindowRect((HWND)hwnd, &rect);
   if(client)
   {
      RECT client; GetClientRect((HWND)hwnd, &client);
      w+=(rect.right -rect.left)-(client.right -client.left);
      h+=(rect.bottom-rect.top )-(client.bottom-client.top );
   }
   MoveWindow((HWND)hwnd, rect.left, rect.top, w, h, true);
}
VecI2 WindowSize(Bool client, Ptr hwnd)
{
   RECT rect;
   if(!client)
   {
      if(!GetWindowRect((HWND)hwnd, &rect))goto error;
   }else
   {
      if(!GetClientRect((HWND)hwnd, &rect))goto error;
   }
   return VecI2(rect.right-rect.left, rect.bottom-rect.top);
error:
   return 0;
}
RectI WindowRect(Bool client, Ptr hwnd) // !! 'WindowRect' can return weird position when the window is minimized !!
{
   RECT rect;
   if(!client)
   {
      if(!GetWindowRect((HWND)hwnd, &rect))goto error;
   }else
   {
      POINT pos={0, 0};
      if(!GetClientRect ((HWND)hwnd, &rect))goto error;
          ClientToScreen((HWND)hwnd, &pos );
      rect.left +=pos.x; rect.top   +=pos.y;
      rect.right+=pos.x; rect.bottom+=pos.y;
   }
#if DEBUG && 0
   LogN(S+"WindowRect("+client+")="+RectI(rect.left, rect.top, rect.right, rect.bottom).asText());
#endif
   return RectI(rect.left, rect.top, rect.right, rect.bottom);
error:
   return RectI(0, 0, 0, 0);
}
Bool WindowMaximized(Ptr hwnd)
{
   return IsZoomed((HWND)hwnd)!=0;
}
Bool WindowMinimized(Ptr hwnd)
{
   return IsIconic((HWND)hwnd)!=0;
}
void WindowSendData(CPtr data, Int size, Ptr hwnd)
{
   if(hwnd && size>=0)
   {
      COPYDATASTRUCT ds;
      ds.dwData=0; // custom ID, not used
      ds.lpData=Ptr(data);
      ds.cbData=size;
      SendMessage((HWND)hwnd, WM_COPYDATA, (WPARAM)App.hwnd(), (LPARAM)&ds);
   }
}
/******************************************************************************/
Ptr WindowActive()
{
   return (Ptr)GetForegroundWindow();
}
Ptr WindowMouse()
{
   POINT cur; if(GetCursorPos(&cur))return (Ptr)WindowFromPoint(cur); return null;
}
Ptr WindowParent(Ptr hwnd)
{
   return (Ptr)GetParent((HWND)hwnd);
}
/******************************************************************************/
static BOOL CALLBACK EnumWindowList(HWND hwnd, LPARAM hwnds_ptr)
{
   MemPtr<Ptr> &hwnds=*(MemPtr<Ptr>*)hwnds_ptr;
   hwnds.add(hwnd);
   return true;
}
void WindowList(MemPtr<Ptr> hwnds)
{
   hwnds.clear();
   EnumWindows(EnumWindowList, LPARAM(&hwnds));
}
/******************************************************************************/
#elif MAC
/******************************************************************************/
void WindowSetText(C Str &text, Ptr hwnd)
{
   if(NSWindow *window=(NSWindow*)hwnd)
   if(NSStringAuto string=text)
      [window setTitle:string];
}
Str WindowGetText(Ptr hwnd)
{
   if(NSWindow *window=(NSWindow*)hwnd)return [window title]; // do not release [window title] as it will crash
   return S;
}
void WindowActivate(Ptr hwnd)
{
   if(NSWindow *window=(NSWindow*)hwnd)
      if(window==App.hwnd())
   {
      ProcessSerialNumber psn;
      if(GetProcessForPID(App.processID(), &psn)==noErr)SetFrontProcess(&psn);
   }
}
Ptr WindowMouse()
{
   return Ms._on_client ? App.hwnd() : null;
}
Bool WindowMaximized(Ptr hwnd)
{
   if(NSWindow *window=(NSWindow*)hwnd)return [window isZoomed];
   return false;
}
Bool WindowMinimized(Ptr hwnd)
{
   if(NSWindow *window=(NSWindow*)hwnd)return [window isMiniaturized];
   return false;
}
void WindowClose(Ptr hwnd)
{
   if(NSWindow *window=(NSWindow*)hwnd)[window performClose:NSApp];
}
void WindowMinimize(Bool force, Ptr hwnd)
{
   if(NSWindow *window=(NSWindow*)hwnd)[window performMiniaturize:NSApp];
}
void WindowMaximize(Bool force, Ptr hwnd)
{
   if(NSWindow *window=(NSWindow*)hwnd)
   {
      if( [window isMiniaturized])WindowActivate(hwnd);
      if(![window isZoomed])[window performZoom:NSApp];
   }
}
void WindowReset(Bool force, Ptr hwnd)
{
   if(NSWindow *window=(NSWindow*)hwnd)
   {
      if([window isMiniaturized])WindowActivate(hwnd);else
         if([window isZoomed])[window performZoom:NSApp];
   }
}
void WindowToggle(Bool force, Ptr hwnd)
{
   if(NSWindow *window=(NSWindow*)hwnd)
   {
      if([window isMiniaturized])WindowActivate(hwnd);else
         [window performZoom:NSApp];
   }
}
void WindowHide(Ptr hwnd)
{
   if(NSWindow *window=(NSWindow*)hwnd)
      if(window==App.hwnd())[NSApp hide:NSApp];
}
void WindowShow(Ptr hwnd)
{
   if(NSWindow *window=(NSWindow*)hwnd)
      if(window==App.hwnd())[NSApp unhideWithoutActivation];
}
static VecI2 WindowSize(Bool client, NSWindow *window)
{
   NSRect    rect=[window frame];
   if(client)rect=[window contentRectForFrameRect:rect];
   return VecI2(Round(rect.size.width), Round(rect.size.height));
}
static void GetWindowRect(Bool client, NSWindow *window, RectI &r)
{
   NSRect    rect=[window frame];
   if(client)rect=[window contentRectForFrameRect:rect];
   r.min.x=            Round(rect.origin.x); r.max.x=r.min.x+Round(rect.size.width );
   r.max.y=D.screenH()-Round(rect.origin.y); r.min.y=r.max.y-Round(rect.size.height);
}
VecI2 WindowSize(Bool client, Ptr hwnd)
{
   if(NSWindow *window=(NSWindow*)hwnd)
   {
      if(hwnd==App.hwnd() && D.full())return D.res();
      return WindowSize(client, window);
   }
   return 0;
}
RectI WindowRect(Bool client, Ptr hwnd)
{
   RectI r;
   if(NSWindow *window=(NSWindow*)hwnd)
   {
      if(hwnd==App.hwnd() && D.full())r.set(0, 0, D.resW(), D.resH());
      else                            GetWindowRect(client, window, r);
   }else r.zero();
   return r;
}
void WindowMove(Int dx, Int dy, Ptr hwnd)
{
   if((dx || dy) && hwnd)
   {
      RectI r=WindowRect(false, hwnd);
      WindowPos(r.min.x+dx, r.min.y+dy, hwnd);
   }
}
void WindowPos(Int x, Int y, Ptr hwnd)
{
   if(NSWindow *window=(NSWindow*)hwnd)
   {
      NSPoint p; p.x=x; p.y=D.screenH()-y;
      [window setFrameTopLeftPoint:p];
   }
}
void WindowSize(Int w, Int h, Bool client, Ptr hwnd)
{
   if(NSWindow *window=(NSWindow*)hwnd)
   {
      // don't use [window setContentSize:NSSize] as it will resize while preserving the bottom left window position
      NSRect rect=[window frame];
      Flt rect_h=rect.size.height;
      rect.size.width =w;
      rect.size.height=h;
      if(client)
      {
      #if 1
         rect.size.width +=App._bound.w();
         rect.size.height+=App._bound.h();
      #else
         rect.size=[window frameRectForContentRect:rect].size;
      #endif
      }
      rect.origin.y+=rect_h-rect.size.height; // 'origin' is bottom-left window position
      [window setFrame:rect display:true]; // 'setFrame' includes borders
   }
}
Ptr WindowParent(Ptr hwnd)
{
   if(NSWindow *window=(NSWindow*)hwnd)return [window parentWindow];
   return null;
}
void WindowFlash(Ptr hwnd)
{
   if(App.hwnd()==hwnd) // on Mac OS only our window can be flashed
      if(NSApplication *app=NSApp) // get current application id
         [app requestUserAttention:NSInformationalRequest];
}
Byte WindowGetAlpha(Ptr hwnd)
{
   if(NSWindow *window=(NSWindow*)hwnd)return FltToByte(window.alphaValue);
   return 255;
}
void WindowAlpha(Byte alpha, Ptr hwnd)
{
   if(NSWindow *window=(NSWindow*)hwnd)[window setAlphaValue:alpha/255.0f];
}
void WindowSendData(CPtr data, Int size, Ptr hwnd) {}
Ptr WindowActive() {return App.active() ? App.hwnd() : null;}
/******************************************************************************/
#elif LINUX
/******************************************************************************/
#define _NET_WM_STATE_REMOVE 0
#define _NET_WM_STATE_ADD    1
#define _NET_WM_STATE_TOGGLE 2

static Atom     xdnd_req, WM_PROTOCOLS, WM_DELETE_WINDOW, XdndDrop, XdndActionCopy, XdndPosition, XdndEnter, XdndStatus, XdndTypeList, XdndFinished, XdndSelection, PRIMARY, WM_STATE, _NET_WM_STATE, _NET_WM_STATE_HIDDEN, _NET_WM_STATE_FOCUSED, _NET_WM_STATE_MAXIMIZED_VERT, _NET_WM_STATE_MAXIMIZED_HORZ, _NET_WM_STATE_FULLSCREEN, _NET_WM_STATE_DEMANDS_ATTENTION, _NET_WM_NAME, _NET_FRAME_EXTENTS, UTF8_STRING, _MOTIF_WM_HINTS;
static XWindow  xdnd_source;
static long     xdnd_version;
static VecI2    xdnd_pos;
static int      XInput2Extension;
static Colormap HwndColormap;
static Byte     ButtonPressCount[8];
static XIM      IM;
static XIC      IC;
static Str8     ClassName;

struct MotifWmHints2
{
   unsigned long flags;
   unsigned long functions;
   unsigned long decorations;
            long input_mode;
   unsigned long status;
};
/******************************************************************************/
void WindowSetText(C Str &text, Ptr hwnd)
{
   if(XDisplay && hwnd)
   {
      Str8 utf=UTF8(text);
                                     XStoreName     (XDisplay, XWindow(hwnd), Str8(text));
      if(_NET_WM_NAME && UTF8_STRING)XChangeProperty(XDisplay, XWindow(hwnd), _NET_WM_NAME, UTF8_STRING, 8, PropModeReplace, (unsigned char*)utf(), utf.length());
   }
}
Str WindowGetText(Ptr hwnd)
{
   Str s;
   if(XDisplay && hwnd)
   {
      if(_NET_WM_NAME && UTF8_STRING)
      {
         Atom           type=NULL;
         int            format=0;
         unsigned long  items=0, bytes_after=0;
         unsigned char *data=null;
         if(!XGetWindowProperty(XDisplay, XWindow(hwnd), _NET_WM_NAME, 0, 4096, false, UTF8_STRING, &type, &format, &items, &bytes_after, &data))s=FromUTF8((char*)data);
         if(data)XFree(data);
      }
      if(!s.is())
      {
         char *name=null; XFetchName(XDisplay, XWindow(hwnd), &name);
         if(name){s=name; XFree(name);}
      }
   }
   return s;
}
void WindowActivate(Ptr hwnd)
{
   if(XDisplay && hwnd)
   {
      XRaiseWindow  (XDisplay, XWindow(hwnd));
      XSetInputFocus(XDisplay, XWindow(hwnd), RevertToParent, CurrentTime);
   }
}
Ptr WindowMouse()
{
   if(XDisplay)
   {
      XWindow root, child;
      int rx, ry, x, y;
      unsigned int mask;
      XQueryPointer(XDisplay, DefaultRootWindow(XDisplay), &root, &child, &rx, &ry, &x, &y, &mask);
      return Ptr(XmuClientWindow(XDisplay, XWindow(WindowParentTop(Ptr(child)))));
   }
   return null;
}
Bool WindowMaximized(Ptr hwnd)
{
   if(XDisplay && hwnd && _NET_WM_STATE)
   {
      UInt           flags=0;
      Atom           type=NULL;
      int            format=0;
      unsigned long  items=0, bytes_after=0;
      unsigned char *data=null;
      if(!XGetWindowProperty(XDisplay, XWindow(hwnd), _NET_WM_STATE, 0, 1024, false, XA_ATOM, &type, &format, &items, &bytes_after, &data))
         if(Atom *atoms=(Atom*)data)
            for(unsigned long i=0; i<items; i++)
            {
               if(atoms[i]==_NET_WM_STATE_MAXIMIZED_HORZ)flags|=1;else
               if(atoms[i]==_NET_WM_STATE_MAXIMIZED_VERT)flags|=2;
            }
      if(data)XFree(data);
      return flags==3;
   }
   return false;
}
Bool WindowMinimized(Ptr hwnd)
{
   if(XDisplay && hwnd && WM_STATE)
   {
      struct State
      {
         CARD32 state;
         XID    icon;
      };
      bool           min=false;
      Atom           type=NULL;
      int            format=0;
      unsigned long  items=0, bytes_after=0;
      unsigned char *data=null;
      if(!XGetWindowProperty(XDisplay, XWindow(hwnd), WM_STATE, 0, SIZE(State)/4, false, WM_STATE, &type, &format, &items, &bytes_after, &data))
         if(State *state=(State*)data)min=(state->state==IconicState);
      if(data)XFree(data);
      return min;
   }
   return false;
}
void WindowClose(Ptr hwnd)
{
   if(XDisplay && hwnd)XDestroyWindow(XDisplay, XWindow(hwnd));
}
void WindowMinimize(Bool force, Ptr hwnd)
{
   if(XDisplay && hwnd)XIconifyWindow(XDisplay, XWindow(hwnd), DefaultScreen(XDisplay));
}
void WindowMaximize(Bool force, Ptr hwnd)
{
   if(XDisplay && hwnd && _NET_WM_STATE && _NET_WM_STATE_MAXIMIZED_HORZ && _NET_WM_STATE_MAXIMIZED_VERT)
   {
   #if 1
      XEvent e; Zero(e);
      e.xclient.type        =ClientMessage;
      e.xclient.window      =XWindow(hwnd);
      e.xclient.message_type=_NET_WM_STATE;
      e.xclient.format      =32;
      e.xclient.data.l[0]=_NET_WM_STATE_ADD;
      e.xclient.data.l[1]=_NET_WM_STATE_MAXIMIZED_HORZ;
      e.xclient.data.l[2]=_NET_WM_STATE_MAXIMIZED_VERT;
      e.xclient.data.l[3]=1;
      e.xclient.data.l[4]=0;
      XSendEvent(XDisplay, DefaultRootWindow(XDisplay), false, SubstructureRedirectMask|SubstructureNotifyMask, &e);
   #else // this doesn't work entirely correctly
      Atom           type=null;
      int            format=0;
      unsigned long  items=0, bytes_after=0;
      unsigned char *data=null;
      if(!XGetWindowProperty(XDisplay, XWindow(hwnd), _NET_WM_STATE, 0, 1024, false, XA_ATOM, &type, &format, &items, &bytes_after, &data))
      {
         Atom *atoms=(Atom*)data, temp[1024];
         if(items<Elms(temp)-2) // room for _NET_WM_STATE_MAXIMIZED_HORZ, _NET_WM_STATE_MAXIMIZED_VERT
         {
            UInt flags=0;
            for(unsigned long i=0; i<items; i++)
            {
               temp[i]=atoms[i];
               if(temp[i]==_NET_WM_STATE_MAXIMIZED_HORZ)flags|=1;else
               if(temp[i]==_NET_WM_STATE_MAXIMIZED_VERT)flags|=2;
            }
            if(flags!=3)
            {
               if(!(flags&1))temp[items++]=_NET_WM_STATE_MAXIMIZED_HORZ;
               if(!(flags&2))temp[items++]=_NET_WM_STATE_MAXIMIZED_VERT;
               XChangeProperty(XDisplay, XWindow(hwnd), _NET_WM_STATE, XA_ATOM, 32, PropModeReplace, (unsigned char*)temp, items);
            }
         }
      }
      if(data)XFree(data);
      XMoveResizeWindow(XDisplay, XWindow(hwnd), 0, 0, App.desktopArea().w(), App.desktopArea().h()); // 'XMoveResizeWindow' accepts client size
   #endif
   }
}
void WindowReset(Bool force, Ptr hwnd)
{
   if(XDisplay && hwnd && _NET_WM_STATE && _NET_WM_STATE_MAXIMIZED_HORZ && _NET_WM_STATE_MAXIMIZED_VERT)
   {
   #if 1
      XEvent e; Zero(e);
      e.xclient.type        =ClientMessage;
      e.xclient.window      =XWindow(hwnd);
      e.xclient.message_type=_NET_WM_STATE;
      e.xclient.format      =32;
      e.xclient.data.l[0]=_NET_WM_STATE_REMOVE;
      e.xclient.data.l[1]=_NET_WM_STATE_MAXIMIZED_HORZ;
      e.xclient.data.l[2]=_NET_WM_STATE_MAXIMIZED_VERT;
      e.xclient.data.l[3]=1;
      e.xclient.data.l[4]=0;
      XSendEvent(XDisplay, DefaultRootWindow(XDisplay), false, SubstructureRedirectMask|SubstructureNotifyMask, &e);
   #else // this doesn't work entirely correctly
      if(hwnd==App.hwnd())
      {
         Atom           type=null;
         int            format=0;
         unsigned long  items=0, bytes_after=0;
         unsigned char *data=null;
         if(!XGetWindowProperty(XDisplay, XWindow(hwnd), _NET_WM_STATE, 0, 1024, false, XA_ATOM, &type, &format, &items, &bytes_after, &data))
         {
            Atom *atoms=(Atom*)data, temp[1024];
            if(items<Elms(temp))
            {
               unsigned long new_items=0;
               for(unsigned long i=0; i<items; i++)
                  if(atoms[i]!=_NET_WM_STATE_MAXIMIZED_HORZ && atoms[i]!=_NET_WM_STATE_MAXIMIZED_VERT)temp[new_items++]=atoms[i];

               if(new_items!=items)XChangeProperty(XDisplay, XWindow(hwnd), _NET_WM_STATE, XA_ATOM, 32, PropModeReplace, (unsigned char*)temp, new_items);
            }
         }
         if(data)XFree(data);
         XMoveResizeWindow(XDisplay, XWindow(hwnd), App._window_pos.x, App._window_pos.y, App._window_size.x, App._window_size.y); // 'XMoveResizeWindow' accepts client size
      }
   #endif
   }
}
void WindowToggle(Bool force, Ptr hwnd)
{
   if(WindowMaximized(hwnd))WindowReset   (force, hwnd);
   else                     WindowMaximize(force, hwnd);
}
void WindowHide(Ptr hwnd)
{
   if(XDisplay && hwnd)XUnmapWindow(XDisplay, XWindow(hwnd));
}
void WindowShow(Ptr hwnd)
{
   if(XDisplay && hwnd)XMapWindow(XDisplay, XWindow(hwnd));
}
VecI2 WindowSize(Bool client, Ptr hwnd)
{
   if(XDisplay && hwnd)
   {
      XWindowAttributes attr; if(XGetWindowAttributes(XDisplay, XWindow(hwnd), &attr)==true)
      {
         if(!client){attr.width+=App._bound.w(); attr.height+=App._bound.h();}
         return VecI2(attr.width, attr.height);
      }
   }
   return 0;
}
RectI WindowRect(Bool client, Ptr hwnd)
{
   if(XDisplay && hwnd)
   {
      XWindowAttributes attr;
   #if 0
      if(!client)hwnd=WindowParentTop(hwnd); // start straight from the top
   #endif
      if(XGetWindowAttributes(XDisplay, XWindow(hwnd), &attr)==true)
      {
         RectI r(attr.x, attr.y, attr.x+attr.width, attr.y+attr.height);
      #if 0
         if(client) // we want just the client
      #endif
            for(; hwnd=WindowParent(hwnd); )if(XGetWindowAttributes(XDisplay, XWindow(hwnd), &attr)==true)r+=VecI2(attr.x, attr.y);
      #if 1
         if(!client){r.min+=App._bound.min; r.max+=App._bound.max;}
      #endif
         return r;
      }
   }
   return RectI(0, 0, 0, 0);
}
void WindowMove(Int dx, Int dy, Ptr hwnd)
{
   if(XDisplay && hwnd && (dx || dy))
   {
      VecI2 pos=WindowRect(false, hwnd).min;
      XMoveWindow(XDisplay, XWindow(hwnd), pos.x+dx, pos.y+dy);
   }
}
void WindowPos(Int x, Int y, Ptr hwnd)
{
   if(XDisplay && hwnd)XMoveWindow(XDisplay, XWindow(hwnd), x, y);
}
void WindowSize(Int w, Int h, Bool client, Ptr hwnd)
{
   if(XDisplay && hwnd)
   {
      if(!client)
      {
      #if 1
         VecI2 border=App._bound.size();
      #else
         VecI2 border=WindowSize(false, hwnd)-WindowSize(true, hwnd);
      #endif
         w-=border.x;
         h-=border.y;
      }
      XResizeWindow(XDisplay, XWindow(hwnd), w, h); // 'XResizeWindow' accepts client size
   }
}
Ptr WindowParent(Ptr hwnd)
{
   if(XDisplay && hwnd)
   {
      XWindow root=NULL, parent=NULL, *children=null;
      unsigned int nchildren=0;
      XQueryTree(XDisplay, XWindow(hwnd), &root, &parent, &children, &nchildren);
      if(children)XFree(children);
      if(root!=parent)return Ptr(parent);
   }
   return null;
}
void WindowFlash(Ptr hwnd)
{
   if(XDisplay && hwnd && _NET_WM_STATE && _NET_WM_STATE_DEMANDS_ATTENTION)
   {
   #if 0 // this doesn't work at all
      XClientMessageEvent event; Zero(event);
      event.type        =ClientMessage;
      event.message_type=_NET_WM_STATE;
      event.display   =XDisplay;
      event.serial    =0;
      event.window    =XWindow(hwnd);
      event.send_event=1;
      event.format   =32;
      event.data.l[0]=_NET_WM_STATE_ADD;
      event.data.l[1]=_NET_WM_STATE_DEMANDS_ATTENTION;
      XSendEvent(XDisplay, DefaultRootWindow(XDisplay), false, SubstructureRedirectMask|SubstructureNotifyMask, (XEvent*)&event);
   #elif 0 // this doesn't work at all
      XEvent e; Zero(e);
      e.xclient.type        =ClientMessage;
      e.xclient.window      =XWindow(hwnd);
      e.xclient.message_type=_NET_WM_STATE;
      e.xclient.format      =32;
      e.xclient.data.l[0]=_NET_WM_STATE_ADD;
      e.xclient.data.l[1]=_NET_WM_STATE_DEMANDS_ATTENTION;
      e.xclient.data.l[2]=0;
      e.xclient.data.l[3]=1;
      XSendEvent(XDisplay, DefaultRootWindow(XDisplay), false, SubstructureRedirectMask|SubstructureNotifyMask, &e);
   #else // more complex code but works
      Atom           type=NULL;
      int            format=0;
      unsigned long  items=0, bytes_after=0;
      unsigned char *data=null;
      if(!XGetWindowProperty(XDisplay, XWindow(hwnd), _NET_WM_STATE, 0, 1024, false, XA_ATOM, &type, &format, &items, &bytes_after, &data))
      {
         Atom *atoms=(Atom*)data, temp[1024];
         if(items<Elms(temp)-1) // room for '_NET_WM_STATE_DEMANDS_ATTENTION'
         {
            bool has=false;
            for(unsigned long i=0; i<items; i++)
            {
               temp[i]=atoms[i];
               if(temp[i]==_NET_WM_STATE_DEMANDS_ATTENTION)has=true;
            }
            if(!has)
            {
               temp[items++]=_NET_WM_STATE_DEMANDS_ATTENTION;
               XChangeProperty(XDisplay, XWindow(hwnd), _NET_WM_STATE, XA_ATOM, 32, PropModeReplace, (unsigned char*)temp, items);
            }
         }
      }
      if(data)XFree(data);
   #endif
   }
}
Byte WindowGetAlpha(Ptr hwnd) {return 255;}
void WindowAlpha(Byte alpha, Ptr hwnd) {}
void WindowSendData(CPtr data, Int size, Ptr hwnd) {}
Ptr  WindowActive()
{
   if(XDisplay)
   {
      XWindow hwnd; int revert_to;
      if(XGetInputFocus(XDisplay, &hwnd, &revert_to)==True)return Ptr(XmuClientWindow(XDisplay, XWindow(WindowParentTop(Ptr(hwnd)))));
   }
   return App.active() ? App.hwnd() : null;
}
/******************************************************************************/
#else
/******************************************************************************/
void WindowSetText(C Str &text, Ptr hwnd)
{
   if(hwnd==App.hwnd())
   {
      App._name=text;
   #if WINDOWS_NEW
      Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->Title=ref new Platform::String(text);
   #endif
   }
}
Str WindowGetText(Ptr hwnd)
{
   if(hwnd==App.hwnd())return App.name();
   return S;
}
void WindowActivate(Ptr hwnd)
{
}
Ptr WindowMouse()
{
   return (App.active() && !App.minimized()) ? App.hwnd() : null;
}
Bool WindowMaximized(Ptr hwnd)
{
   return hwnd==App.hwnd() && App.maximized();
}
Bool WindowMinimized(Ptr hwnd)
{
   return hwnd==App.hwnd() && App.minimized();
}
void WindowClose(Ptr hwnd)
{
}
void WindowMinimize(Bool force, Ptr hwnd)
{
#if WINDOWS_NEW
   //if(hwnd==App.hwnd())Windows::ApplicationModel::Core::CoreApplication::Exit(); this causes a crash, TODO: find alternative
#elif ANDROID
   if(hwnd==App.hwnd() && AndroidApp && AndroidApp->activity)ANativeActivity_finish(AndroidApp->activity);
#endif
}
void WindowMaximize(Bool force, Ptr hwnd)
{
}
void WindowReset(Bool force, Ptr hwnd)
{
}
void WindowToggle(Bool force, Ptr hwnd)
{
}
void WindowHide(Ptr hwnd)
{
   WindowMinimize(false, hwnd);
}
void WindowShow(Ptr hwnd)
{
   WindowReset(false, hwnd);
}
VecI2 WindowSize(Bool client, Ptr hwnd)
{
   if(hwnd==App.hwnd())
   {
   #if WINDOWS_NEW
      if(App.hwnd())
      {
         Windows::Foundation::Rect rect=App.Hwnd()->Bounds; // this returns the client rect
         return VecI2(DipsToPixels(rect.Width), DipsToPixels(rect.Height));
      }
   #endif
      return D.res();
   }
   return 0;
}
RectI WindowRect(Bool client, Ptr hwnd)
{
   if(hwnd==App.hwnd())
   {
   #if WINDOWS_NEW
      if(App.hwnd())
      {
         Windows::Foundation::Rect rect=App.Hwnd()->Bounds; // this returns the client rect
         return RectI(DipsToPixels(rect.X), DipsToPixels(rect.Y), DipsToPixels(rect.X+rect.Width), DipsToPixels(rect.Y+rect.Height));
      }
   #endif
      return RectI(0, 0, D.resW(), D.resH());
   }
   return RectI(0, 0, 0, 0);
}
void WindowMove(Int dx, Int dy, Ptr hwnd)
{
}
void WindowPos(Int x, Int y, Ptr hwnd)
{
}
void WindowSize(Int w, Int h, Bool client, Ptr hwnd)
{
}
Ptr WindowParent(Ptr hwnd)
{
   return null;
}
void WindowFlash(Ptr hwnd)
{
}
Byte WindowGetAlpha(Ptr hwnd) {return 255;}
void WindowAlpha(Byte alpha, Ptr hwnd) {}
void WindowSendData(CPtr data, Int size, Ptr hwnd) {}
Ptr WindowActive() {return App.active() ? App.hwnd() : null;}
/******************************************************************************/
#endif
#if WINDOWS_NEW
   // TODO: WINDOWS_NEW TaskBar Progress - check this in the future as right now this is not available in UWP
void WindowSetNormal  (              Ptr hwnd) {}
void WindowSetWorking (              Ptr hwnd) {}
void WindowSetProgress(Flt progress, Ptr hwnd) {}
void WindowSetPaused  (Flt progress, Ptr hwnd) {}
void WindowSetError   (Flt progress, Ptr hwnd) {}
#elif !WINDOWS
void WindowSetNormal  (              Ptr hwnd) {}
void WindowSetWorking (              Ptr hwnd) {}
void WindowSetProgress(Flt progress, Ptr hwnd) {}
void WindowSetPaused  (Flt progress, Ptr hwnd) {}
void WindowSetError   (Flt progress, Ptr hwnd) {}
#endif
UInt WindowProc(Ptr hwnd)
{
#if WINDOWS_OLD
   DWORD proc=0; GetWindowThreadProcessId((HWND)hwnd, &proc); return proc;
#else
   if(hwnd==App.hwnd())return App.processID();
   return 0;
#endif
}
#if !WINDOWS_OLD
void WindowList(MemPtr<Ptr> hwnds)
{
   if(App.hwnd())hwnds.setNum(1)[0]=App.hwnd();else hwnds.clear();
}
#endif
/******************************************************************************/
Ptr WindowParentTop(Ptr hwnd)
{
   for(; Ptr parent=WindowParent(hwnd); )hwnd=parent; return hwnd;
}
/******************************************************************************/
void WindowMsgBox(C Str &title, C Str &text, Bool error)
{
#if WINDOWS_OLD
   MessageBox(null, text, title, MB_OK|MB_TOPMOST|(error ? MB_ICONERROR : 0)); // this does not require 'FixNewLine'
#elif WINDOWS_NEW
   if(auto dialog=ref new Windows::UI::Popups::MessageDialog(ref new Platform::String(text), ref new Platform::String(title))) // this does not require 'FixNewLine'
   {
      dialog->Commands->Append(ref new Windows::UI::Popups::UICommand("OK"));
      dialog->ShowAsync();
   }
#elif LINUX // TODO: what if zenity is not installed?
   Str safe_title=          Str(title).replace('`', '\'').replace('"', '\'');
   Str safe_text =XmlString(Str(text ).replace('`', '\''));
   Run("zenity", S+(error ? "--error" : "--info")+" --title=\""+safe_title+"\" --text=\""+safe_text+"\"");
#elif MAC
   CFStringRef	cf_title=              CFStringCreateWithCString(kCFAllocatorDefault, title.is() ? UTF8(title)() : "", kCFStringEncodingUTF8); // 'CFUserNotificationDisplayAlert' will freeze if this param is null
   CFStringRef	cf_text =(text .is() ? CFStringCreateWithCString(kCFAllocatorDefault,              UTF8(text )       , kCFStringEncodingUTF8) : null);
   CFUserNotificationDisplayAlert(0, error ? kCFUserNotificationStopAlertLevel : kCFUserNotificationNoteAlertLevel, null, null, null, cf_title, cf_text, CFSTR("OK"), null, null, null);
   if(cf_title)CFRelease(cf_title);
   if(cf_text )CFRelease(cf_text );
#elif IOS
	if(NSString *ns_title=AppleString(title)) // have to use 'AppleString' because it will get copied in the local function below
   {
   	if(NSString *ns_text=AppleString(text)) // have to use 'AppleString' because it will get copied in the local function below
      {
         dispatch_async(dispatch_get_main_queue(), ^{ // this is needed in case we're calling from a secondary thread
            if(UIAlertController *alert_controller=[UIAlertController alertControllerWithTitle:ns_title message:ns_text preferredStyle:UIAlertControllerStyleAlert])
            {
               [alert_controller addAction:[UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:nil]];
               [[[[UIApplication sharedApplication] keyWindow] rootViewController] presentViewController:alert_controller animated:YES completion:nil];
             //[alert_controller release]; release will crash
            }
         });
         [ns_text release];
      }
      [ns_title release];
   }
#elif ANDROID
   // we need to call the code on UI thread, so we need to call java that will do this
   JNI jni;
   if(jni && ActivityClass)
   if(JMethodID messageBox=jni->GetStaticMethodID(ActivityClass, "messageBox", "(Ljava/lang/String;Ljava/lang/String;Z)V"))
      if(JString ti=JString(jni, title))
      if(JString te=JString(jni, text ))
         jni->CallStaticVoidMethod(ActivityClass, messageBox, ti(), te(), jboolean(false));
#elif WEB
   JavaScriptRun(S+"alert(\""+CString(text)+"\")"); 
#endif
}
/******************************************************************************/
#if WINDOWS_OLD
static void UpdateCandidates() // this function does not remove exising candidates unless it founds new ones (this is because candidates for "q6" keyboard input on QuanPin vista/7 would get cleared)
{
   Memc<Str> &candidate=(Kb._imm_candidate_hidden ? Kb._imm_candidate_temp : Kb._imm_candidate); candidate.clear();
   Int size=ImmGetCandidateList(Kb._imc, 0, null, 0);
   if( size>0)
   {
      Memt<Byte> temp; CANDIDATELIST *list=(CANDIDATELIST*)temp.setNumZero(size).data(); ImmGetCandidateList(Kb._imc, 0, list, size);
      if(list->dwStyle==IME_CAND_CODE)
      {
         if(list->dwCount==1)candidate.New()=(Char)list->dwOffset[0];else
         if(list->dwCount> 1){} // text representations of individual DBCS character values in hexadecimal notation ?
      }else
      if(list->dwPageSize)
      {
         Int start=Max((Int)list->dwPageStart,       (Int)(list->dwSelection/list->dwPageSize*list->dwPageSize)          ), // must be aligned to page size
             end  =Min((Int)list->dwCount    , start+(Int) list->dwPageSize                                    , start+10); // limit to 10 elements
         for(Int i=start; i<end; i++)
         {
            CChar *c=WChar((LPTSTR)(((UIntPtr)list)+list->dwOffset[i]));
            if(Is(c))candidate.add(c);else break;
         }
      }
   }
}

static Bool Updating;

static SByte Activate=-1;
static Bool  NonClientClick=false, ErasedBackground=false;

enum PAUSE_MODE : Byte
{
   NOT_PAUSED,
   PAUSED_WAS_INACTIVE,
   PAUSED_WAS_ACTIVE,
   PAUSED_TIMER,
};
static PAUSE_MODE PauseMode=NOT_PAUSED;
static UIntPtr    PauseTimer;
static void       Pause(Bool pause)
{
   if(pause!=(PauseMode!=NOT_PAUSED))switch(PauseMode)
   {
      case NOT_PAUSED: // is not paused, pause now
      {
         if(App.flag&APP_NO_PAUSE_ON_WINDOW_MOVE_SIZE)
         {
            Time.skipUpdate(); PauseTimer=SetTimer(App.Hwnd(), 1, USER_TIMER_MINIMUM, null); PauseMode=PAUSED_TIMER;
         }else
         {
            PauseMode=(App.active() ? PAUSED_WAS_ACTIVE : PAUSED_WAS_INACTIVE); App.setActive(false);
         }
      }break;

      case PAUSED_TIMER: KillTimer(App.Hwnd(), PauseTimer); PauseTimer=0; PauseMode=NOT_PAUSED; break;

      case PAUSED_WAS_ACTIVE  : App.setActive(true); // !! no break on purpose !!
      case PAUSED_WAS_INACTIVE: PauseMode=NOT_PAUSED; break;
   }
}

static Byte ResetCursorCounter;
static void ResetCursor() {Ms.resetCursor(); if(ResetCursorCounter){ResetCursorCounter--; App._callbacks.include(ResetCursor);}} // calling immediately may not have any effect, we have to try a few times

static void ConditionalDraw()
{
   if(!(App.active() || (App.flag&APP_WORK_IN_BACKGROUND)))DrawState(); // draw only if will not draw by itself
}
static LRESULT CALLBACK WindowMsg(HWND hwnd, UInt msg, WPARAM wParam, LPARAM lParam)
{
#if 0
   switch(msg)
   {
      case WM_KEYDOWN:
      case WM_KEYUP:

      case WM_MOUSEMOVE:
      case WM_MBUTTONUP:
      case WM_MOUSEWHEEL:

      case WM_NCPAINT:
      case WM_NCHITTEST:
    //case WM_NCCALCSIZE:

      case WM_ERASEBKGND:
      case WM_SETCURSOR:
      case WM_GETMINMAXINFO:
      case WM_PAINT:

      case WM_GETICON:
      case WM_GETTEXT:
      case WM_GETOBJECT: break;
      default:
      {
         Str s=S+"frame:"+Time.frame()+", WM_";
         switch(msg)
         {
            case WM_POWERBROADCAST: s+=S+"POWERBROADCAST:"+wParam; break;
            case WM_DISPLAYCHANGE: s+=S+"DISPLAYCHANGE:"+LOWORD(lParam)+'x'+HIWORD(lParam); break;
            case WM_ACTIVATE: s+=S+"ACTIVATE:"+(wParam==WA_ACTIVE || wParam==WA_CLICKACTIVE); break;
            case WM_NCACTIVATE: s+=S+"NCACTIVATE"; break;
            case WM_ACTIVATEAPP: s+=S+"ACTIVATEAPP"; break;
            case WM_MOVE: s+=S+"MOVE:"+(short)LOWORD(lParam)+','+(short)HIWORD(lParam); break;
            case WM_SIZE: s+=S+"SIZE:"+LOWORD(lParam)+','+HIWORD(lParam); break;
            case WM_NCHITTEST: s+=S+"NCHITTEST"; break;
            case WM_NCCALCSIZE: s+=S+"NCCALCSIZE"; break;
            case WM_SETFOCUS: s+=S+"SETFOCUS"; break;
            case WM_KILLFOCUS: s+=S+"KILLFOCUS"; break;
            case WM_DWMNCRENDERINGCHANGED: s+=S+"DWMNCRENDERINGCHANGED"; break;
            case WM_SETTINGCHANGE:
            {
               s+=S+"SETTINGCHANGE: SPI_";
               switch(wParam)
               {
                  default: s+=TextHex(wParam); break;
                  case SPI_SETWORKAREA: {RECT rect; SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0); s+=S+"SETWORKAREA:"+rect.left+','+rect.top+','+rect.right+','+rect.bottom;} break;
               }
            }break;
            case WM_WINDOWPOSCHANGING: {WINDOWPOS &wp=*(WINDOWPOS*)lParam; s+=S+"WINDOWPOSCHANGING:"+wp.x+','+wp.y+' '+wp.cx+','+wp.cy;} break;
            case WM_WINDOWPOSCHANGED : {WINDOWPOS &wp=*(WINDOWPOS*)lParam; s+=S+"WINDOWPOSCHANGED:" +wp.x+','+wp.y+' '+wp.cx+','+wp.cy;} break;
            case WM_STYLECHANGING: s+=S+"STYLECHANGING"; break;
            case WM_STYLECHANGED: s+=S+"STYLECHANGED"; break;
            default: s+=TextHex(msg)+", wParam:"+TextHex(wParam)+", lParam:"+TextHex((ULong)lParam); break;
         }
         LogN(s);
      }break;
   }
#endif
   switch(msg)
   {
      // TIMER
      case WM_TIMER: if(wParam==PauseTimer && !App._close && !Updating)App.update(); break; // allows app updating while window moving/resizing (don't do this if app requested close or it's inside update already, this can happen if in the app we've displayed a system message box making the app paused, and then moving the window, in that case WM_TIMER gets called on the same thread, however inside the messagebox call)

      // APPLICATION
      case WM_CLOSE: if(!(App.flag&APP_NO_CLOSE))App.close(); return 0;

    /*case WM_ACTIVATEAPP: // !! Warning: when clicking on the apps bar in the taskbar to minimize it, WM_ACTIVATEAPP gets called with "wParam==1", but when clicking on the apps bar in the taskbar to activate it, WM_ACTIVATEAPP does not get called !! that's why WM_ACTIVATEAPP is ignored and only WM_ACTIVATE gets checked
      {
         Bool active=(wParam!=0); // 'wParam' can be 0 or 1
         App.setActive(active);
       //LogN(S+"WM_ACTIVATEAPP, active:"+active+", wParam:"+wParam);
      }break;*/

      case WM_ACTIVATE: // !! Warning: when clicking on the apps bar in the taskbar to minimize it, WM_ACTIVATE gets called with "LOWORD(wParam)!=WA_INACTIVE" !!
      {
       //Bool active=(LOWORD(wParam)!=WA_INACTIVE), minimized=(HIWORD(wParam)!=0); active&=!minimized; // LOWORD(wParam) can be: WA_INACTIVE, WA_ACTIVE, WA_CLICKACTIVE. HIWORD(wParam)!=0 indicates being minimized
         Activate=(wParam==WA_ACTIVE || wParam==WA_CLICKACTIVE); // it's active only for WA_ACTIVE WA_CLICKACTIVE and when HIWORD(wParam)==0, which is "active && !minimized"
         // instead of activating the app here, we need to wait to check if WM_NCLBUTTONDOWN gets called to determine whether app was activated with a click on the title bar
      }break;

      case WM_INITMENUPOPUP  : case WM_ENTERSIZEMOVE: Pause(true ); break;
      case WM_UNINITMENUPOPUP: case WM_EXITSIZEMOVE : Pause(false); break; // WM_EXITSIZEMOVE called when (finished dragging by title bar or resizing by edge/corner, snapped by User), NOT called when (maximized, snapped by OS)

      case WM_MOVE: // called when moved (dragged by title bar, snapped by User/OS, maximized, minimized)
         if(!WindowMinimized(hwnd) && !WindowMaximized(hwnd) && !D.full()) // use 'hwnd' instead of 'App.hwnd' because WM_MOVE is being called while window is being created "_hwnd=CreateWindowEx(..)" and pointer wasn't set yet, need to check for 'WindowMinimized' and 'WindowMaximized' instead of 'App.minimized' and 'App.maximized' because these are not yet available
      {
       //VecI2 client((short)LOWORD(lParam), (short)HIWORD(lParam));
         App._window_pos=WindowRect(false, hwnd).min; // remember window position for later restoring
      }break;

      case WM_SIZE: // called when resized (resized by edge/corner, snapped by User/OS, maximized, minimized)
      {
         App._minimized=(wParam==SIZE_MINIMIZED);
         App._maximized=(wParam==SIZE_MAXIMIZED);
         if(!App.minimized() && !D.full() && D.created())
         {
            App._window_resized.set(LOWORD(lParam), HIWORD(lParam));
            ConditionalDraw(); // draw too because WM_PAINT won't be called when window is getting smaller
         }
      }break;

      case WM_WINDOWPOSCHANGED: // called when moved/resized (dragged by title bar, resized by edge/corner, snapped by User/OS, maximized, minimized)
         Ms.clipUpdate();
      break;

      case WM_DISPLAYCHANGE: if(D.initialized()) // needed only if device already initialized (to skip setting mouse cursor and screen size when initializing)
      {
         ResetCursorCounter=8; App._callbacks.include(ResetCursor); // it was noticed that after changing resolution, Windows will rescale current cursor, to prevent that, we need to reset it, calling immediately may not have any effect, we have to try a few times
         if(auto screen_changed=D.screen_changed)screen_changed(D.w(), D.h()); // if 'D.scale' is set based on current screen resolution, then we may need to adjust it
      }break;

      case WM_SYSCOMMAND: switch(wParam)
      {
         case SC_CLOSE       : if(!(App.flag&APP_NO_CLOSE))App.close(); return 0;
         case SC_MONITORPOWER: return 0;
         case SC_SIZE        : return 0;
         case SC_MINIMIZE    : if(!(App.flag&APP_MINIMIZABLE))return 0; break;
         case SC_MAXIMIZE    : if(!(App.flag&APP_MAXIMIZABLE))return 0; break;

         case SC_MOVE        :
         case SC_KEYMENU     : if(D.full())return 0; break;
      }break;

      // POWER
      case WM_POWERBROADCAST: switch(wParam)
      {
         case PBT_APMQUERYSUSPEND: return App._stay_awake ? BROADCAST_QUERY_DENY : true; // system asks if it's OK to sleep

         case PBT_APMSUSPEND        : // suspending
         case PBT_APMRESUMEAUTOMATIC: // resuming
         {
            if(auto sleep=App.sleep)sleep(wParam==PBT_APMSUSPEND); // copy first to avoid multi-thread issues
         }break;
      }break;

    //case WM_DPICHANGED: break;

      // MOUSE, because there can be a case when the Window is activated by System through WM_ACTIVATE, but we don't activate the App due to Ms.exclusive or Ms.clip, then we always need to activate when the user clicks on the client area
      case WM_LBUTTONDOWN: App.setActive(true); Ms.push   (0); return 0;   case WM_RBUTTONDOWN: Ms.push   (1); return 0;   case WM_MBUTTONDOWN: Ms.push   (2); return 0;
      case WM_LBUTTONUP  :                      Ms.release(0); return 0;   case WM_RBUTTONUP  : Ms.release(1); return 0;   case WM_MBUTTONUP  : Ms.release(2); return 0;
      case WM_XBUTTONDOWN: Ms.push   ((GET_XBUTTON_WPARAM(wParam)&XBUTTON1) ? 3 : 4); return 0;
      case WM_XBUTTONUP  : Ms.release((GET_XBUTTON_WPARAM(wParam)&XBUTTON1) ? 3 : 4); return 0;

      case WM_NCLBUTTONDOWN: // when clicking on the title bar, this will get called before WM_ACTIVATE, but when clicking on minimize/maximize/close, then it will get called after
      {
       //LogN(S+"WM_NCLBUTTONDOWN, frame:"+Time.frame());
         NonClientClick=true;
         Time.skipUpdate(); // pause can occur when holding mouse button over title bar or window buttons for less time than move/size gets activated, thus causing slow down, so call 'skipUpdate'
      }break;
    //case WM_NCLBUTTONUP: break; this is never called

      case WM_SETCURSOR: // this will get called only if the mouse is on the app window, there will be different results if mouse is on client or for example on the title bar
      {
         if(Ms._on_client=(LOWORD(lParam)==HTCLIENT)){Ms.resetVisibility(); return true;} // call this here to make sure that we have correct cursor set, in Mouse.update there's also '_on_client' modification, because this isn't called when mouse goes outside the window, return true only when on client, so for example at the window edge, system resize cursor can be assigned by default functions
      }break;

      #define WM_MOUSEHWHEEL 0x020E
      case    WM_MOUSEHWHEEL: Ms._wheel.x+=Flt(GET_WHEEL_DELTA_WPARAM(wParam))/WHEEL_DELTA; break;
      case    WM_MOUSEWHEEL : Ms._wheel.y+=Flt(GET_WHEEL_DELTA_WPARAM(wParam))/WHEEL_DELTA; break;

      // KEYBOARD
      // Order of events 0-WM_INPUT, 1-WM_KEYDOWN, 2-WM_CHAR, 3-WM_KEYUP
      case WM_INPUT:
      {
         UINT size=0; GetRawInputData((HRAWINPUT)lParam, RID_INPUT, null, &size, sizeof(RAWINPUTHEADER));
         Memt<Byte> temp; temp.setNum(size);
         if(GetRawInputData((HRAWINPUT)lParam, RID_INPUT, temp.data(), &size, sizeof(RAWINPUTHEADER))==size)
         {
            RAWINPUT &raw=*(RAWINPUT*)temp.data(); switch(raw.header.dwType)
            {
               case RIM_TYPEKEYBOARD:
               {
                  KB_KEY key;
                  switch(raw.data.keyboard.VKey)
                  {
                     case VK_CONTROL: if(raw.data.keyboard.Flags&RI_KEY_E0)goto def; key=KB_LCTRL; break; // skip RI_KEY_E0 right control (it's already handled in WM_KEYDOWN)
                     case VK_SHIFT  : key=((raw.data.keyboard.MakeCode==42) ? KB_LSHIFT : KB_RSHIFT); break; // 42=KB_LSHIFT, 54=KB_RSHIFT
                     case 255       : if(raw.data.keyboard.MakeCode==42 && (raw.data.keyboard.Flags&(RI_KEY_E0|RI_KEY_E1))==RI_KEY_E0){key=KB_PRINT; break;} goto def; // detect KB_PRINT because in 'Kb.exclusive', WM_HOTKEY isn't called
                     default: goto def;
                  }
                  if(raw.data.keyboard.Flags&RI_KEY_BREAK)Kb.release(key);else Kb.push(key, raw.data.keyboard.MakeCode);
                  return 0;
               }break;

               case RIM_TYPEMOUSE:
               {
                  if(raw.data.mouse.usFlags&MOUSE_MOVE_ABSOLUTE)
                  {
                  }else
                  {
                     Ms._delta_relative.x+=raw.data.mouse.lLastX;
                     Ms._delta_relative.y-=raw.data.mouse.lLastY;
                  }
               }break;
            }
         }
      }break;

      case WM_KEYDOWN   :
      case WM_SYSKEYDOWN: // SYSKEYDOWN handles Alt+keys
      {
      #if DEBUG
         U16  rep=lParam&0xFFFF;
         Bool ext=(lParam>>24)&1,
              ctx_code=(lParam>>29)&1,
              prev_state=(lParam>>30)&1,
             trans_state=(lParam>>31)&1;
      #endif
         Byte   scan_code=(lParam>>16)&0xFF;
         KB_KEY key=KB_KEY(wParam);
         switch(key)
         {
            case KB_CTRL :    if(lParam&(1<<24))key=KB_RCTRL;else return 0; break; // can't push KB_LCTRL, because it could be triggered by KB_RALT, just ignore this rely on RawInput/DirectInput
          //case KB_SHIFT: key=((lParam&(1<<24)) ?  KB_RSHIFT : KB_LSHIFT); break; this is not working OK, lParam&(1<<24) is always false
          //case KB_SHIFT: key=((scan_code==42 ) ?  KB_LSHIFT : KB_RSHIFT); break; 42=KB_LSHIFT, 54=KB_RSHIFT, releasing doesn't work OK
          //case KB_SHIFT: key=(KB_KEY)MapVirtualKey((lParam>>16)&0xFF, MAPVK_VSC_TO_VK_EX); break; releasing doesn't work OK
            case KB_ALT  : key=((lParam&(1<<24)) ?  KB_RALT   : KB_LALT  ); break;
         }
         Kb.push(key, scan_code);
         // !! queue characters after push !!
         if(Kb.anyCtrl() && !Kb.anyAlt()) // if Control is on, then WM_CHAR will not be called, so we must add this char here, don't do this with Alt pressed, because if Ctrl+Alt are pressed, then accented characters will get generated (even if it's left Alt)
         {
            if(key>='A' && key<='Z')Kb.queue(Char(key + (Kb.anyShift() ? 0 : 'a'-'A')), scan_code);else
            if(key>='0' && key<='9')Kb.queue(Char(key)                                , scan_code);
         }
      }return 0;

      case WM_SYSCHAR: // SYSCHAR handles Alt+chars, also disables beep on Alt+key menu sound when keyboard in non exclusive mode
      case WM_CHAR   :
      {
      #if DEBUG
         U16  rep=lParam&0xFFFF;
         Bool ext=(lParam>>24)&1,
              ctx_code=(lParam>>29)&1,
              prev_state=(lParam>>30)&1,
             trans_state=(lParam>>31)&1;
      #endif
         Byte scan_code=(lParam>>16)&0xFF;
      #ifdef UNICODE
         Kb.queue((Char)wParam, scan_code);
      #else
         Kb.queue(Char8To16Fast(wParam), scan_code); // we can assume that Str was already initialized
      #endif
      }return 0;

      case WM_KEYUP   :
      case WM_SYSKEYUP: // for KB_SHIFT this will be called only if both shifts are released
      {
      #if DEBUG
         Byte   scan_code=(lParam>>16)&0xFF;
      #endif
         KB_KEY key=KB_KEY(wParam);
         switch(key)
         {
            case KB_CTRL : if(lParam&(1<<24))key=KB_RCTRL;else
                           #if KB_RAW_INPUT
                              return 0;
                           #else
                              if(Kb._special&1){key=KB_LCTRL; FlagDisable(Kb._special, 1);}else return 0; // release LCTRL only if it was locked
                           #endif
            break;
          //case KB_SHIFT: key=((lParam&(1<<24)) ? KB_RSHIFT : KB_LSHIFT); break; this is not working OK, lParam&(1<<24) is always false
          //case KB_SHIFT: key=((scan_code==42 ) ? KB_LSHIFT : KB_RSHIFT); break; 42=KB_LSHIFT, 54=KB_RSHIFT, will not be called for one Shift key if other is already pressed
          //case KB_SHIFT: key=(KB_KEY)MapVirtualKey((lParam>>16)&0xFF, MAPVK_VSC_TO_VK_EX); break; will not be called for one Shift key if other is already pressed
            case KB_ALT  : key=((lParam&(1<<24)) ? KB_RALT   : KB_LALT  ); break;
         }
         Kb.release(key);
      }return 0;

      case WM_HOTKEY:
      {
      #if !KB_RAW_INPUT
         if((lParam>>16)==VK_SNAPSHOT)Kb.push(KB_PRINT, -1); // this is needed only for DirectInput non-exclusive mode
      #endif
      }break;

      // IME
      case WM_INPUTLANGCHANGE     : Kb.setLayout(); break; // LANG_TYPE((lParam>>16)&0xFF)
      case WM_IME_CHAR            :                                    return 0; // don't process by OS
      case WM_IME_KEYUP           : if(!D.exclusive())break;           return 0; // don't process by OS
      case WM_IME_KEYDOWN         : if(!D.exclusive())break;           return 0; // don't process by OS
      case WM_IME_SETCONTEXT      : if(!D.exclusive())break; lParam=0; break   ; // disables drawing of some windows, "lParam=0" disables drawing candidate list by OS, and enables accessing it manually using GetCandidateList
      case WM_IME_REQUEST         : if(!D.exclusive())break;           return 0; // don't process by OS
      case WM_IME_SELECT          : if(!D.exclusive())break;           return 0; // don't process by OS
      case WM_IME_COMPOSITIONFULL : if(!D.exclusive())break;           return 0; // don't process by OS
      case WM_IME_STARTCOMPOSITION: if(!D.exclusive())break;           return 0; // don't process by OS
      case WM_IME_ENDCOMPOSITION  : if(!D.exclusive())break;           return 0; // don't process by OS
      case WM_IME_COMPOSITION     :
      {
         Kb._imm_buffer.clear(); // always 'clear' even for 'reserve' to avoid copying old data in 'setNum'
         Kb._imm_cursor  =LOWORD(ImmGetCompositionString(Kb._imc, GCS_CURSORPOS, null, 0));
         Int chars_needed=       ImmGetCompositionString(Kb._imc, GCS_COMPSTR  , null, 0)/SIZE(Char);
         if( chars_needed>0)
         {
            Kb._imm_buffer.reserve(chars_needed); ImmGetCompositionString(Kb._imc, GCS_COMPSTR, Kb._imm_buffer._d.data(), chars_needed*SIZE(Char));
            Kb._imm_buffer._d     [chars_needed]=0;
            Kb._imm_buffer._length=Length(Kb._imm_buffer());
         }

         Kb._imm_selection=-1;
         Int clause_size=ImmGetCompositionStringA(Kb._imc, GCS_COMPCLAUSE, null, 0); // use A because W crashes for GCS_COMPCLAUSE under WinXP
         if( clause_size>0)
         {
            Memc<UInt> clause; clause.setNum(clause_size/SIZE(UInt)); ImmGetCompositionStringA(Kb._imc, GCS_COMPCLAUSE, clause.data(), clause.elms()*clause.elmSize()); // use A because W crashes for GCS_COMPCLAUSE under WinXP
            FREPA(clause)if(clause[i]>=Kb.immCursor())
            {
               Kb._imm_selection=clause[i];
               if(InRange(i+1, clause))Kb._imm_selection.y=clause[i+1];
               break;
            }
         }
         if(lParam&GCS_RESULTSTR)
         {
            Int chars_needed=                                                     ImmGetCompositionString(Kb._imc, GCS_RESULTSTR, null      ,                       0)/SIZE(Char);
            if( chars_needed>0){Memt<Char, 8*1024> imm; imm.setNum(chars_needed); ImmGetCompositionString(Kb._imc, GCS_RESULTSTR, imm.data(), chars_needed*SIZE(Char)); FREP(chars_needed)Kb.queue(imm[i], -1);}
         }

      #if SUPPORT_WINDOWS_XP // additional check for WinXP
         if(OSVerNumber().x<=5) // WinXP and below (WinXP is 5.1, Vista is 6.0, Win7 is 6.1, Win8 is 6.2, Win8.1 is 6.3, Win10 is 10)
            UpdateCandidates(); // must be called here as well because of Chinese NeiMa on WinXP (NeiMa doesn't call IMN_CHANGECANDIDATE), however this can't be called for Vista or newer because it causes disappearing of candidates for QuanPin when typing "q6"
      #endif

         if(!D.exclusive())break;
      }return 0; // don't process by OS

      case WM_IME_NOTIFY:
      {
         switch(wParam)
         {
            case IMN_OPENCANDIDATE  : if( Kb._imm_candidate_hidden){Kb._imm_candidate_hidden=false; Swap(Kb._imm_candidate_temp, Kb._imm_candidate);} break; // open  must "show" candidates
            case IMN_CLOSECANDIDATE : if(!Kb._imm_candidate_hidden){Kb._imm_candidate_hidden=true ; Swap(Kb._imm_candidate_temp, Kb._imm_candidate);} break; // close must "hide" candidates
            case IMN_CHANGECANDIDATE: UpdateCandidates(); break;
         }
         if(!D.exclusive())break;
      }return 0; // don't process by OS

      // TOUCH
      case WM_POINTERDOWN :
      case WM_POINTERENTER:
      {
         POINT  point={GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}; VecI2 posi(point.x, point.y); ScreenToClient(App.Hwnd(), &point);
         UInt   id=GET_POINTERID_WPARAM(wParam);
         CPtr  pid=CPtr(id);
         Bool   stylus=false; if(GetPointerType){POINTER_INPUT_TYPE type=PT_POINTER; if(GetPointerType(id, &type))if(type==PT_PEN)stylus=true;}
         Vec2   pos=D.windowPixelToScreen(VecI2(point.x, point.y));
         Touch *touch=FindTouchByHandle(pid);
         if(   !touch)touch=&Touches.New().init(posi, pos, pid, stylus);else
         {
            touch->_remove=false; // disable 'remove' in case it was enabled (for example the same touch was released in same/previous frame)
            if(msg!=WM_POINTERENTER)touch->reinit(posi, pos); // re-initialize for push (don't do this for hover because it can be called the same frame that release is called, and for release we want to keep the original values)
         }
         if(msg!=WM_POINTERENTER){touch->_state=BS_ON|BS_PUSHED; touch->_force=1;}
      }return 0; // don't process by OS

      case WM_POINTERUPDATE:
      {
         UInt  id=GET_POINTERID_WPARAM(wParam);
         CPtr pid=CPtr(id);
         if(Touch *touch=FindTouchByHandle(pid))
         {
            POINT point={GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}; VecI2 posi(point.x, point.y); ScreenToClient(App.Hwnd(), &point);
            touch->_deltai+=posi-touch->_posi;
            touch->_posi   =posi;
            touch->_pos    =D.windowPixelToScreen(VecI2(point.x, point.y));
         }
      }return 0; // don't process by OS

      case WM_POINTERUP   :
      case WM_POINTERLEAVE:
      {
         UInt  id=GET_POINTERID_WPARAM(wParam);
         CPtr pid=CPtr(id);
         if(Touch *touch=FindTouchByHandle(pid))
         {
            POINT point={GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}; VecI2 posi(point.x, point.y); ScreenToClient(App.Hwnd(), &point);
            touch->_deltai+=posi-touch->_posi;
            touch->_posi   =posi;
            touch->_pos    =D.windowPixelToScreen(VecI2(point.x, point.y));
            touch->_remove =true;
            if(touch->_state&BS_ON) // check for state in case it was manually eaten
            {
               touch->_state|= BS_RELEASED;
               touch->_state&=~BS_ON;
            }
         }
      }return 0; // don't process by OS

      case WM_POINTERCAPTURECHANGED:
      {
         UInt  id=GET_POINTERID_WPARAM(wParam);
         CPtr pid=CPtr(id);
         if(Touch *touch=FindTouchByHandle(pid))
         {
            touch->_remove=true;
            if(touch->_state&BS_ON) // check for state in case it was manually eaten
            {
               touch->_state|= BS_RELEASED;
               touch->_state&=~BS_ON;
            }
         }
      }return 0; // don't process by OS

      // DRAW
      case WM_PAINT: ConditionalDraw(); break;

      case WM_ERASEBKGND: if(ErasedBackground)return 0; ErasedBackground=true; break;

      // DROP
      case WM_DROPFILES: if(App.drop)
      {
         HDROP     handle=HDROP(wParam);
         wchar_t   name[MAX_LONG_PATH];
         Memc<Str> names; for(Int i=0; DragQueryFile(handle, i++, name, Elms(name))>0; )names.add(name);
         POINT     point; DragQueryPoint(handle, &point); Vec2 pos=D.windowPixelToScreen(VecI2(point.x, point.y)); // get precise drop position
         DragFinish(handle); // release before the callback
         App.drop(names, Gui.objAtPos(pos), pos);
         if(!D.full() && !(App.flag&APP_WORK_IN_BACKGROUND))DrawState();
         return 0;
      }break;

      // RECEIVE DATA
      case WM_COPYDATA: if(App.receive_data)
      {
         if(COPYDATASTRUCT *ds=(COPYDATASTRUCT*)lParam)App.receive_data(ds->lpData, ds->cbData, (Ptr)wParam);
      }break;

      // DEVICES
      case WM_DEVICECHANGE:
      {
         if(wParam==DBT_DEVICEARRIVAL
         || wParam==DBT_DEVICEREMOVECOMPLETE
         || wParam==DBT_DEVNODES_CHANGED)ListJoypads();
      }break;
   }
def:
   return DefWindowProc(hwnd, msg, wParam, lParam);
}
#endif
/******************************************************************************/
#if WINDOWS
static BOOL CALLBACK EnumResources(HMODULE hModule, LPCWSTR lpType, LPWSTR lpName, LONG_PTR lParam)
{
   *((C wchar_t**)lParam)=lpName;
   return false;
}
static ATOM WindowClass=0;
#elif LINUX
struct XProp
{
   Int  format, count;
   Atom type;
   Ptr  data;
};
static Byte Deactivate=0; // after how many frames to deactivate
static void ReadProperty(XProp &p, XWindow w, Atom prop)
{
   unsigned char *ret=null;
   Atom type=NULL;
   int  fmt =0;
   unsigned long count=0;
   unsigned long bytes_left=0;
   int           bytes_fetch = 0;

   do{
      if(ret){XFree(ret); ret=null;}
      XGetWindowProperty(XDisplay, w, prop, 0, bytes_fetch, false, AnyPropertyType, &type, &fmt, &count, &bytes_left, &ret);
      bytes_fetch+=bytes_left;
   }while(bytes_left);

   p.data  =ret;
   p.format=fmt;
   p.count =count;
   p.type  =type;
}
static Atom PickTarget(Atom list[], int list_count)
{
   Atom request=NULL;
   for(int i=0; i<list_count && !request; i++)
   {
      char *name=XGetAtomName(XDisplay, list[i]);
      if(Equal(name, "text/uri-list"))request=list[i];
      XFree(name);
   }
   return request;
}
static Atom PickTargetFromAtoms(Atom a0, Atom a1, Atom a2)
{
   int  count=0;
   Atom atom[3];
   if(a0)atom[count++]=a0;
   if(a1)atom[count++]=a1;
   if(a2)atom[count++]=a2;
   return PickTarget(atom, count);
}
static void GetWindowBounds(XSetWindowAttributes &win_attr, XVisualInfo *vis_info, XWindow root_win)
{
   if(XDisplay)
      if(Atom FIND_ATOM(_NET_REQUEST_FRAME_EXTENTS))
         if(XWindow hwnd=XCreateWindow(XDisplay, root_win, 0, 0, 256, 256, 0, vis_info->depth, InputOutput, vis_info->visual, CWBackPixmap|CWBorderPixel|CWColormap|CWEventMask, &win_attr))
   {
      XEvent event; Zero(event);
      event.xclient.type        =ClientMessage;
      event.xclient.message_type=_NET_REQUEST_FRAME_EXTENTS;
      event.xclient.display     =XDisplay;
      event.xclient.window      =hwnd;
      event.xclient.format      =32;

      XSendEvent(XDisplay, root_win, false, SubstructureRedirectMask|SubstructureNotifyMask, &event);
      XSync(XDisplay, false); // TODO: test on Ubuntu newer than 14.10 if multiple attempts are still needed (14.10 fails always)
      REP(1024) // 1024 attempts
      {
         Atom type=NULL;
         int  format=0;
         unsigned long  items=0, bytes_after=0;
         unsigned char *data=null;
         if(!XGetWindowProperty(XDisplay, hwnd, _NET_FRAME_EXTENTS, 0, SIZE(unsigned long)*4, false, XA_CARDINAL, &type, &format, &items, &bytes_after, &data))
            if(type==XA_CARDINAL && format==32 && items>=4)if(long *l=(long*)data)
         {
            long left  =l[0],
                 right =l[1],
                 top   =l[2],
                 bottom=l[3];
            App._bound.set(-left, -top, right, bottom);
         }
         if(data){XFree(data); break;}
         usleep(1);
      }
      XDestroyWindow(XDisplay, hwnd);
   }
}
static Bool InitXInput2()
{
   int event, error;
   if(XQueryExtension(XDisplay, "XInputExtension", &XInput2Extension, &event, &error))
   {
      int major=2, minor=2; // 2.2 for multi touch
      if(!XIQueryVersion(XDisplay, &major, &minor))
      {
         if(Compare(VecI2(major, minor), VecI2(2, 2))>=0) // got the version
         {
            unsigned char mask[3]={0,0,0};
            XIEventMask event_mask; Zero(event_mask);
            event_mask.deviceid=XIAllMasterDevices;
            event_mask.mask_len=SIZE(mask);
            event_mask.mask    =mask;
            XISetMask(mask, XI_RawMotion);
            XISetMask(mask, XI_RawButtonPress);
            XISetMask(mask, XI_RawButtonRelease);
            if(!XISelectEvents(XDisplay, DefaultRootWindow(XDisplay), &event_mask, 1))
            {
               return true;
            }
         }
      }
   }
   return false;
}
static void SetXInputValues(C Dbl *input, unsigned char *mask, Int mask_len, Dbl *output, Int output_elms)
{
   const Int max_axis=16;
   Int mask_elms=Min(max_axis, mask_len*8); // bits per byte
   ZeroN(output, output_elms);
   for(int i=0, o=0; i<mask_elms && o<output_elms; i++, o++)if(XIMaskIsSet(mask, i))output[o]=*input++;
}
void Application::setWindowFlags(Bool force_resizable)
{
   if(XDisplay && hwnd() && _MOTIF_WM_HINTS)
   {
      force_resizable|=FlagTest(flag, APP_RESIZABLE);
      MotifWmHints2 hints; Zero(hints);
      hints.flags      =MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
      hints.functions  =MWM_FUNC_MOVE |       ((flag&APP_NO_CLOSE)?0:MWM_FUNC_CLOSE) | ((flag&APP_MINIMIZABLE)?MWM_FUNC_MINIMIZE :0) | ((flag&APP_MAXIMIZABLE)?MWM_FUNC_MAXIMIZE :0) | (force_resizable?MWM_FUNC_RESIZE  :0);
      hints.decorations=((flag&APP_NO_TITLE_BAR)?0:MWM_DECOR_BORDER|MWM_DECOR_TITLE) | ((flag&APP_MINIMIZABLE)?MWM_DECOR_MINIMIZE:0) | ((flag&APP_MAXIMIZABLE)?MWM_DECOR_MAXIMIZE:0) | (force_resizable?MWM_DECOR_RESIZEH:0);
      XChangeProperty(XDisplay, Hwnd(), _MOTIF_WM_HINTS, _MOTIF_WM_HINTS, 32, PropModeReplace, (unsigned char*)&hints, SIZE(hints)/4);
   }
}
#endif
void Application::windowCreate()
{
   if(LogInit)LogN("Application.windowCreate");
#if WINDOWS_OLD
   Bool want_minmax = FlagTest(flag, APP_MINIMIZABLE|APP_MAXIMIZABLE),
        want_close  =!FlagTest(flag, APP_NO_CLOSE                   ),
        want_buttons=(want_minmax || want_close);

 C wchar_t *icon=null;
   if(!_icon)EnumResourceNames(_hinstance, RT_GROUP_ICON, EnumResources, (LONG_PTR)&icon); // _hinstance must be used, only this supports loading icon from DLL (GetModuleHandle(null) and null didn't work)

   WNDCLASS wc; Zero(wc);
   Str class_name=S+"Esenthel|"+name()+'|'+(Ptr)_hinstance+'|'+DateTime().getUTC().asText()+'|'+Random(); // create a unique class name, this was needed so that 2 apps don't use the same name because some issues could occur
   wc.style        =((want_buttons && !want_close) ? CS_NOCLOSE : 0);
   wc.lpfnWndProc  =WindowMsg;
   wc.hInstance    =_hinstance;
   wc.hIcon        =(_icon ? _icon : LoadIcon(_hinstance, icon));
   wc.hCursor      =LoadCursor(null, IDC_ARROW);
   wc.hbrBackground=HBRUSH(GetStockObject(BLACK_BRUSH));
   wc.lpszClassName=class_name;
   WindowClass=RegisterClass(&wc);

  _style_window          =(WS_VISIBLE | ((flag&APP_NO_TITLE_BAR)?WS_POPUP:WS_CAPTION) | (want_buttons?WS_SYSMENU:0) | ((flag&APP_MINIMIZABLE)?WS_MINIMIZEBOX:0) | ((flag&APP_MAXIMIZABLE)?WS_MAXIMIZEBOX:0) | ((flag&APP_RESIZABLE)?WS_THICKFRAME:0)              );
  _style_window_maximized=(WS_VISIBLE | ((flag&APP_NO_TITLE_BAR)?WS_POPUP:WS_CAPTION) | (want_buttons?WS_SYSMENU:0) | ((flag&APP_MINIMIZABLE)?WS_MINIMIZEBOX:0) | ((flag&APP_MAXIMIZABLE)?WS_MAXIMIZEBOX:0) | ((flag&APP_RESIZABLE)?WS_THICKFRAME:0) | WS_MAXIMIZE);
  _style_full            =(WS_VISIBLE | (                        WS_POPUP           ) | (want_buttons?WS_SYSMENU:0)                                                                                                                                               );

   {
      RECT rect;
      Zero(rect); AdjustWindowRect(&rect, _style_window          , false); _bound          .set(rect.left, rect.top, rect.right, rect.bottom);
      Zero(rect); AdjustWindowRect(&rect, _style_window_maximized, false); _bound_maximized.set(rect.left, rect.top, rect.right, rect.bottom);
   }
#elif MAC
   // set some initial values, we will change them later after we have bounds
   NSRect rect;
   rect.origin.x   =0;
   rect.origin.y   =0;
   rect.size.width =512;
   rect.size.height=256;

   UInt style=NSWindowStyleMaskTitled;
   if(  flag& APP_MINIMIZABLE               )style|=NSWindowStyleMaskMiniaturizable;
   if(  flag&(APP_MAXIMIZABLE|APP_RESIZABLE))style|=NSWindowStyleMaskResizable; // on Mac this needs to be enabled if we want to have any of APP_MAXIMIZABLE|APP_RESIZABLE, because without it, the maximize button will be hidden and window will not be resizable around the edges
   if(!(flag& APP_NO_CLOSE)                 )style|=NSWindowStyleMaskClosable;

   NSWindow *window=[[MyWindow alloc] initWithContentRect:rect styleMask:style backing:NSBackingStoreBuffered defer:NO];
  _hwnd=window;

   if(!(flag&APP_MINIMIZABLE))[[window standardWindowButton:NSWindowMiniaturizeButton] setHidden:true];
   if(!(flag&APP_MAXIMIZABLE))[[window standardWindowButton:NSWindowZoomButton       ] setHidden:true];
   if(  flag&APP_NO_CLOSE    )[[window standardWindowButton:NSWindowCloseButton      ] setHidden:true];

   WindowSetText(name());
   [window setAcceptsMouseMovedEvents:YES];

   // we can calculate bounds only after having a window
   {
      RectI w, c; GetWindowRect(false, window, w); GetWindowRect(true, window, c);
     _bound.set(w.min.x-c.min.x, w.min.y-c.min.y, w.max.x-c.max.x, w.max.y-c.max.y);
     _bound_maximized=_bound;
   }
#elif LINUX
   if(!XDisplay)return;

   ClassName=S+"Esenthel|"+name()+'|'+DateTime().getUTC().asText()+'|'+Random(); // create a unique class name in case it is needed

   // GL Config
   int attribs[]=
   {
      GLX_X_RENDERABLE , true,
      GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
      GLX_RENDER_TYPE  , GLX_RGBA_BIT,
      GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
      GLX_RED_SIZE     , 8,
      GLX_GREEN_SIZE   , 8,
      GLX_BLUE_SIZE    , 8,
      GLX_ALPHA_SIZE   , 8,
      GLX_DEPTH_SIZE   , 24,
      GLX_STENCIL_SIZE , 8,
      GLX_DOUBLEBUFFER , true,
      NULL
   };
   int count=0; if(GLXFBConfig *fbc=glXChooseFBConfig(XDisplay, DefaultScreen(XDisplay), attribs, &count))
   {
      if(count>=1)GLConfig=fbc[0];
      XFree(fbc);
   }
   if(!GLConfig)Exit("Couldn't find valid GL Config");
   
   // window
   XVisualInfo *vis_info=glXGetVisualFromFBConfig(XDisplay, GLConfig); if(!vis_info)Exit("Couldn't get XVisualInfo");
   XWindow      root_win=RootWindow(XDisplay, DefaultScreen(XDisplay));
   XSetWindowAttributes win_attr; Zero(win_attr);
   win_attr.event_mask=
      StructureNotifyMask                   // needed for ConfigureNotify, MapNotify
            |KeyPressMask|KeyReleaseMask    // needed for KeyPress, KeyRelease
       //|KeymapStateMask                   // needed for KeymapNotify (currently disabled)
         |ButtonPressMask|ButtonReleaseMask // needed for ButtonPress, ButtonRelease
         |FocusChangeMask                   // needed for FocusOut, FocusIn
         |EnterWindowMask|LeaveWindowMask   // needed for EnterNotify, LeaveNotify
   ; // there are others that could be investigated
   win_attr.background_pixmap=NULL;
   win_attr.background_pixel =0;
   win_attr.border_pixel     =0;
   win_attr.colormap         =XCreateColormap(XDisplay, root_win, vis_info->visual, AllocNone);
   if(!(HwndColormap=win_attr.colormap))Exit("Can't create ColorMap");
   if(!(flag&APP_NO_TITLE_BAR))
   {
     _bound.set(-1, -38, 1, 1); // estimate first
      GetWindowBounds(win_attr, vis_info, root_win);
   }
   InitXInput2();
#endif

#if WINDOWS_NEW
   if(flag&APP_NO_TITLE_BAR)Windows::ApplicationModel::Core::CoreApplication::GetCurrentView()->TitleBar->ExtendViewIntoTitleBar=true;
  _window_size=WindowSize(true); // we can't specify a custom '_window_size' because here the Window has already been created, instead obtain what we've got
   Bool change_size=(D.res()!=_window_size);
   RequestDisplayMode(change_size ? D.resW() : -1, change_size ? D.resH() : -1, (D.full()!=T.Fullscreen()) ? D.full() : -1);
#elif MOBILE
   D._full=true; // mobile are always fullscreen
#elif WEB
   D._full=false; // we can never start a WEB app in full mode, it must be entered on user-click only
   Int w=D.resW(), h=D.resH();
#else
   RectI full, work; VecI2 max_normal_win_client_size, maximized_win_client_size;
   D.curMonitor(full, work, max_normal_win_client_size, maximized_win_client_size);
   Int  x=0, y=0, w, h;
   Bool maximize=false;

  _window_size.set(Min(maximized_win_client_size.x, RoundPos(full.w()*0.5f)),
                   Min(maximized_win_client_size.y, RoundPos(full.h()*0.5f)));
   if(D.resW()>=full.w() && D.resH()>=full.h())D._full=true;
   if(D.full())
   {
      if(!D.resW())D._res.x=full.w(); w=full.w();
      if(!D.resH())D._res.y=full.h(); h=full.h();
     _window_pos=INT_MAX;
   }else
   {
      if(!D.resW())D._res.x=_window_size.x;
      if(!D.resH())D._res.y=_window_size.y;
      if( D.resW()>=Min(maximized_win_client_size.x, max_normal_win_client_size.x+1)
       && D.resH()>=Min(maximized_win_client_size.y, max_normal_win_client_size.y+1) && !(flag&APP_HIDDEN)) // if exceeds the limits of a normal window (and is not going to be hidden, we can't create hidden maximized window, because maximizing requires separate call to 'WindowMaximize' which shows window)
      {
         maximize=true;
         D._res.x=maximized_win_client_size.x;
         D._res.y=maximized_win_client_size.y;
      }else
      {
         MIN(D._res.x, max_normal_win_client_size.x);
         MIN(D._res.y, max_normal_win_client_size.y);
        _window_size=D.res();
      }
      w=D.resW();
      h=D.resH();
      Int W=w+(maximize ? _bound_maximized.w() : _bound.w()), // width  including border
          H=h+(maximize ? _bound_maximized.h() : _bound.h()); // height including border
   #if WINDOWS
      if(maximize) // on Windows when wanting to maximize, create the window initially smaller, so when we un-maximize (by clicking the maximize button) it will set this size, however to get maximized state at the start, after window is created, we will maximize it with 'WindowMaximize'
      { // here we calculate the un-maximized size
         W=_window_size.x+_bound.w(); // width  including border
         H=_window_size.y+_bound.h(); // height including border
      }
      // Windows expects the size to include borders
      w=W;
      h=H;
   #endif
      if(T.x<=-1)x=work.min.x+_bound.min.x+1;else if(!T.x)x=work.centerXI()-W/2;else x=work.max.x-W+_bound.max.x-1;
      if(T.y>= 1)y=work.min.y               ;else if(!T.y)y=work.centerYI()-H/2;else y=work.max.y-H+_bound.max.y-1;
   #if !WINDOWS // on Windows set the position according to the smaller size (and not the code below), because it will get positioned in 'WindowMaximize'
      if(maximize) // when wanting to create a window maximized, force position to left top corner
      { // on Windows when window is maximized, client X is at 0, so the window position is moved to the left by border width, and we have to move up too
         x=work.min.x+_bound_maximized.min.x; // this sets client X at 0
         y=work.min.y-_bound_maximized.max.y; // move window up by border width (use bottom and not the top, because top includes title bar, it's bigger)
      }
   #endif
     _window_pos.set(x, y);
   }
#endif

#if WINDOWS_OLD
   UInt style=(D.full() ? _style_full : maximize ? _style_window_maximized : _style_window); if((flag&APP_HIDDEN) || maximize)FlagDisable(style, WS_VISIBLE); // if maximizing then create as hidden, so small sized window will not be displayed, it will be shown either way because below 'WindowMaximize' shows the window
       _hwnd=(Ptr)CreateWindowEx(drop ? WS_EX_ACCEPTFILES : 0, (LPCWSTR)WindowClass, name(), style, x, y, w, h, null, null, _hinstance, null);
   if(!_hwnd)Exit(MLTC(u"Can't create window", PL,u"Nie można utworzyć okienka"));
   if(maximize)WindowMaximize(true);

   // IMM
   Kb._imc=ImmGetContext(Hwnd());
   Kb. imm(false); // disable by default

   // allow drag and drop when ran as admin (without this, drag and drop won't work for admin)
   if(HMODULE user=GetModuleHandle(L"User32.dll"))
   {
      GetPointerType=(decltype(GetPointerType))GetProcAddress(user, "GetPointerType"); // available on Windows 8+
   #if SUPPORT_WINDOWS_XP
      if(BOOL (WINAPI *ChangeWindowMessageFilter)(UINT message, DWORD dwFlag)=(decltype(ChangeWindowMessageFilter))GetProcAddress(user, "ChangeWindowMessageFilter")) // available on Vista+
   #endif
      {
         #define MSGFLT_ADD        1
         #define WM_COPYGLOBALDATA 0x0049
         ChangeWindowMessageFilter(WM_DROPFILES     , MSGFLT_ADD);
         ChangeWindowMessageFilter(WM_COPYDATA      , MSGFLT_ADD);
         ChangeWindowMessageFilter(WM_COPYGLOBALDATA, MSGFLT_ADD);
      }
      if(HPOWERNOTIFY (WINAPI *RegisterSuspendResumeNotification)(HANDLE hRecipient, DWORD Flags)=(decltype(RegisterSuspendResumeNotification))GetProcAddress(user, "RegisterSuspendResumeNotification")) // available on Win8+
         PowerNotify=RegisterSuspendResumeNotification(Hwnd(), DEVICE_NOTIFY_WINDOW_HANDLE);
   }
   RegisterHotKey(Hwnd(), 0, 0, VK_SNAPSHOT); // allows KB_PRINT detection through WM_HOTKEY, and disable system shortcut, WM_KEYDOWN does not detect KB_PRINT, however WM_KEYUP does detect it. WM_HOTKEY won't work in KB_RAW_INPUT 'Kb.exclusive', but still call this, in case exclusive==false, to disable system screenshot slow downs
#elif MAC
   WindowPos (x, y);
   WindowSize(w, h, true);

   // get rect after setting final window size
   rect=[window contentRectForFrameRect:[window frame]];
   OpenGLView=[[MyOpenGLView alloc] initWithFrame:rect];

   NSTrackingArea *tracking_area=[[NSTrackingArea alloc] initWithRect:rect options:(NSTrackingActiveAlways|NSTrackingMouseEnteredAndExited|NSTrackingInVisibleRect) owner:OpenGLView userInfo:nil];
   [OpenGLView addTrackingArea:tracking_area]; // needed for 'mouseEntered, mouseExited'
   [tracking_area release];

   if([OpenGLView respondsToSelector:@selector(setWantsBestResolutionOpenGLSurface:)])[OpenGLView setWantsBestResolutionOpenGLSurface:YES];
   [window setContentView:OpenGLView];
   [OpenGLView registerForDraggedTypes:[NSArray arrayWithObject:NSFilenamesPboardType]]; // enable drag and drop
#elif LINUX
  _hwnd=(Ptr)XCreateWindow(XDisplay, root_win, x, y, w, h, 0, vis_info->depth, InputOutput, vis_info->visual, CWBackPixmap|CWBorderPixel|CWColormap|CWEventMask, &win_attr);
   if(!_hwnd)Exit("Can't create window");
   WindowSetText(name());
   XSetWindowBackground(XDisplay, Hwnd(), 0);

   if(FIND_ATOM(WM_PROTOCOLS))
   if(FIND_ATOM(WM_DELETE_WINDOW))XSetWMProtocols(XDisplay, Hwnd(), &WM_DELETE_WINDOW, true); // register custom "close window" callback

   setWindowFlags();

   if(drop)if(Atom FIND_ATOM(XdndAware))
   {
      int xdnd_version=5; XChangeProperty(XDisplay, Hwnd(), XdndAware, XA_ATOM, 32, PropModeReplace, (unsigned char*)&xdnd_version, 1);
   }
   FIND_ATOM(XdndDrop);
   FIND_ATOM(XdndActionCopy);
   FIND_ATOM(XdndPosition);
   FIND_ATOM(XdndEnter);
   FIND_ATOM(XdndStatus);
   FIND_ATOM(XdndTypeList);
   FIND_ATOM(XdndFinished);
   FIND_ATOM(XdndSelection);
   FIND_ATOM(PRIMARY);

   CChar8 *class_name=ClassName();
   XWindow hwnd      =Hwnd();
   if(IM=XOpenIM(XDisplay, null, (char*)class_name, (char*)class_name))
      IC=XCreateIC(IM, XNClientWindow, hwnd, XNFocusWindow, hwnd, XNInputStyle, XIMPreeditNothing|XIMStatusNothing, XNResourceName, class_name, XNResourceClass, class_name, Ptr(null)); // last parameter must be of Ptr type and not Int

   if(_icon.is())icon(_icon);

   if(!(flag&APP_HIDDEN))XMapWindow(XDisplay, Hwnd()); // display window after everything is ready
#elif WEB
   Flt zoom=D.browserZoom();
   if(w<=0 || h<=0) // if dimensions haven't been specified
   {
      if(flag&APP_WEB_DISABLE_AUTO_RESIZE)
      {
         Dbl css_w, css_h; emscripten_get_element_css_size(null, &css_w, &css_h); // first try getting from CSS
         w=RoundPos(css_w*zoom); h=RoundPos(css_h*zoom);
         if(w<=0 || h<=0) // if still unavailable
         {
            emscripten_get_canvas_element_size(null, &w, &h);
            if(w<=0 || h<=0){w=800; h=600;} // if still unavailable then use some constant
         }
      }else
      {
      #if 0 // can't use this because the values are always Int's
         w=Max(1, RoundPos(TextFlt(JavaScriptRunS("window.innerWidth" ))*zoom));
         h=Max(1, RoundPos(TextFlt(JavaScriptRunS("window.innerHeight"))*zoom));
      #else
         JavaScriptRun("var target=Module['canvas'];if(target){target.style.width='100vw';target.style.height='100vh';}"); // resize to max
         Dbl css_w, css_h; emscripten_get_element_css_size(null, &css_w, &css_h); // get actual size
         w=Max(1, RoundPos(css_w*zoom)); h=Max(1, RoundPos(css_h*zoom)); // calculate pixels
       //LogN(S+css_w+' '+css_h+' '+css_w*zoom+' '+css_h*zoom+' '+w+' '+h);
      #endif
      }
   }
   Vec2 css_size=VecI2(w, h)/zoom; // calculate css
   emscripten_set_element_css_size   (null, css_size.x, css_size.y); // this accepts floating point sizes
   emscripten_set_canvas_element_size(null, w, h);
#endif
}
void Application::windowDel()
{
#if WINDOWS_OLD
   if(Kb._imc){ImmReleaseContext(Hwnd(), Kb._imc); Kb._imc=null;}
   if(PowerNotify)
   {
      if(HMODULE user=GetModuleHandle(L"User32.dll"))
         if(BOOL (WINAPI *UnregisterSuspendResumeNotification)(HPOWERNOTIFY Handle)=(decltype(UnregisterSuspendResumeNotification))GetProcAddress(user, "UnregisterSuspendResumeNotification")) // available on Win8+
            UnregisterSuspendResumeNotification(PowerNotify);
      PowerNotify=null;
   }
   if(_hwnd){DestroyWindow(Hwnd()); _hwnd=null;}
   if(_icon){DestroyIcon  (_icon ); _icon=null;}
   UnregisterClass((LPCWSTR)WindowClass, _hinstance);
#elif WINDOWS_NEW
#elif MAC
   [OpenGLView release]; OpenGLView=null;
   [    Hwnd() release]; _hwnd     =null;
#elif LINUX
   if( IC          ){XDestroyIC    (IC                    );  IC          =null;}
   if( IM          ){XCloseIM      (IM                    );  IM          =null;}
   if(_hwnd        ){XDestroyWindow(XDisplay, Hwnd      ()); _hwnd        =null;}
   if( HwndColormap){XFreeColormap (XDisplay, HwndColormap);  HwndColormap=NULL;}
#else
   WindowClose(hwnd());
#endif
  _hwnd=null;
}
NOINLINE void Application::windowMsg() // disable inline so we will don't use its stack memory, so we can have more memory for application
{
#if LINUX
   if(!XDisplay)return;
#endif

#if MAC
   NSDate *untilDate=null;
#else
   Bool wait_end_set=false; UInt wait_end;
#endif

   if(active_wait && active()) // check 'active_wait' first, because it's most likely zero (rare case)
   {
      if(active_wait<0) // unlimited wait
      {
      #if WINDOWS_OLD
         WaitMessage();
      #elif WINDOWS_NEW
         ExecuteRecordedEvents(); // since we're going to process system loop, first we need to execute any previously recorded events
         Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher->ProcessEvents(Windows::UI::Core::CoreProcessEventsOption::ProcessOneAndAllPending);
      #elif MAC
         [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantFuture] inMode:NSDefaultRunLoopMode dequeue:NO]; // 'distantFuture' will wait for an event
      #elif LINUX
         goto process; // jump to 'XNextEvent' which will block until event occurs
      #endif
      }else // finite wait
      {
      #if WINDOWS_OLD
         MsgWaitForMultipleObjects(0, null, false, active_wait, QS_ALLINPUT);
      #elif WINDOWS_NEW
         // TODO: have to use global "Bool EventOccured" in callbacks, combined with Time.wait(1) ? but that would slow down
      #elif MAC
         [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate dateWithTimeIntervalSinceNow:(active_wait/1000.0)] inMode:NSDefaultRunLoopMode dequeue:NO];
      #elif LINUX
         if(!XEventsQueued(XDisplay, QueuedAfterFlush)) // no events
         {
            wait_end=Time.curTimeMs()+active_wait; // calc end-time
            do Time.wait(1);while(!XEventsQueued(XDisplay, QueuedAfterFlush) // no events
                               && Signed(wait_end-Time.curTimeMs())>0);
         }
      #endif
      }
   }

start:

#if WINDOWS_OLD
   for(MSG msg; PeekMessage(&msg, null, 0, 0, PM_REMOVE); )
   {
      TranslateMessage(&msg);
       DispatchMessage(&msg);
   }
   if(Activate>=0)
   {
      if(!(NonClientClick && (Ms._clip_rect_on || Ms._clip_window) && Activate)) // if just clicked on the title bar (including Window Min/Max/Close buttons) and the mouse is going to (disappear or get clipped) and we were going to activate, then do nothing, this so that the user can Minimize/Maximize/Close, because otherwise, the mouse cursor would disappear or got clipped and the action wouldn't be performed
         setActive(Activate!=0);
      NonClientClick=false;
      Activate=-1;
   }
#elif WINDOWS_NEW
   ExecuteRecordedEvents(); // since we're going to process system loop, first we need to execute any previously recorded events
   Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher->ProcessEvents(Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent);
again:
#elif MAC
   for(; NSEvent *event=[NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES]; ) // 'distantPast' will not wait for any new events but return those that happened already
      [NSApp sendEvent:event];
#elif LINUX
   for(; XEventsQueued(XDisplay, QueuedAfterFlush); )
   {
   process:
      XEvent event; XNextEvent(XDisplay, &event);
      if(event.xany.window==Hwnd())switch(event.type)
      {
         case MapNotify:
         {
            if(_NET_FRAME_EXTENTS)
            {
               Atom           type  =NULL;
               int            format=0;
               unsigned long  items =0, bytes_after=0;
               unsigned char *data  =null;
               if(!XGetWindowProperty(XDisplay, Hwnd(), _NET_FRAME_EXTENTS, 0, 16, 0, XA_CARDINAL, &type, &format, &items, &bytes_after, &data))
                  if(type==XA_CARDINAL && format==32 && items>=4)if(long *l=(long*)data)               
               {
                  long left  =l[0],
                       right =l[1],
                       top   =l[2],
                       bottom=l[3];
                 _bound.set(-left, -top, right, bottom);
               }
               if(data)XFree(data);
            }
         }break;

         case ConfigureNotify:
         {
           _window_resized.set(event.xconfigure.width, event.xconfigure.height);
           _maximized     =WindowMaximized();
            // minimized is not available here
         }break;

         case FocusOut: Deactivate=2; break; // when pressing Alt Key, FocusOut/FocusIn may get called multiple times within 0-1 frames, to avoid this, we record the event and deactivate after a delay, during which the app may get 'FocusIn' which will cancel this
         case FocusIn : Deactivate=0; setActive(true); XSetICFocus(IC); break; // disable 'Deactivate'

         case EnterNotify: Ms._on_client=true ; Ms.resetVisibility(); break;
         case LeaveNotify: Ms._on_client=false; Ms.resetVisibility(); break;
         
         case KeyPress:
         {
            KeyCode    code=event.xkey.keycode;
            if(InRange(code, ScanCodeToKey))
            {
               KB_KEY  key=ScanCodeToKey[code];
               Kb.push(key, code);
               // !! queue characters after push !!
               if(Kb.anyCtrl()) // if control is pressed then 'chr' is not a character but below 32
               {
                  if(key>='A' && key<='Z')Kb.queue(Char(key + (Kb.anyShift() ? 0 : 'a'-'A')), code);else
                  if(key>='0' && key<='9')Kb.queue(Char(key                                ), code);
               }
            }
            // !! queue characters after push !!
            if(IC)
            {
               wchar_t chr[256];
               KeySym  symbol=0;
               int     status;
               int     pressed=XwcLookupString(IC, &event.xkey, chr, Elms(chr), &symbol, &status);
                     //code   =XKeysymToKeycode(XDisplay, symbol);
               FREP(pressed){Char c=chr[i]; if(c!=127)Kb.queue(c, code);}
            }else
            {
               char           chr[256];
               KeySym         symbol=0;
               XComposeStatus status;
               int            pressed=XLookupString(&event.xkey, chr, Elms(chr), &symbol, &status);
                            //code   =XKeysymToKeycode(XDisplay, symbol);
               FREP(pressed){Char c=chr[i]; if(c!=127)Kb.queue(c, code);}
            }
         }break;

         case KeyRelease:
         {
            char           chr;
            KeySym         symbol=0;
            XComposeStatus status;
            XLookupString(&event.xkey, &chr, 1, &symbol, &status);
            KeyCode        code=event.xkey.keycode; // XKeysymToKeycode(XDisplay, symbol);
            if(InRange(code, ScanCodeToKey))
            {
               char keymap[32]; XQueryKeymap(XDisplay, keymap); // on Linux 'KeyRelease' may get called multiple times in case the key is still held on, so actual button state (keymap) must be checked
               if(!(keymap[code>>3]&(1<<(code&7))))Kb.release(ScanCodeToKey[code]); // if actually released
            }
         }break;

         case ButtonPress:
         {
            switch(event.xbutton.button)
            {
               case 1: Ms. push  (0); break; // LMB
               case 2: Ms. push  (2); break; // MMB
               case 3: Ms. push  (1); break; // RMB
               case 4: Ms._wheel.y++; break;
               case 5: Ms._wheel.y--; break;                                      
               case 6: Ms._wheel.x--; break;
               case 7: Ms._wheel.x++; break;
            }
         }break;

         case ButtonRelease:
         {
            switch(event.xbutton.button)
            {
               case 1: Ms.release(0); break; // LMB
               case 2: Ms.release(2); break; // MMB
               case 3: Ms.release(1); break; // RMB
            }
         }break;
         
         case ClientMessage:
         {
            if(event.xclient.message_type==WM_PROTOCOLS)
            {
               if(event.xclient.data.l[0]==WM_DELETE_WINDOW)
               {
                  if(!(flag&APP_NO_CLOSE))close();
               }
            }else
            if(event.xclient.message_type==XdndEnter)
            {
               bool use_list=(event.xclient.data.l[1]&1);
               xdnd_source  = event.xclient.data.l[0];
               xdnd_version =(event.xclient.data.l[1]>>24);
               if(use_list)
               {
                  XProp p; ReadProperty(p, xdnd_source, XdndTypeList);
                  xdnd_req=PickTarget((Atom*)p.data, p.count);
                  XFree(p.data);
               }else
               {
                  xdnd_req=PickTargetFromAtoms(event.xclient.data.l[2], event.xclient.data.l[3], event.xclient.data.l[4]);
               }
            }else
            if(event.xclient.message_type==XdndPosition)
            {
               XClientMessageEvent m; Zero(m);
               m.type   =ClientMessage;
               m.display=event.xclient.display;
               m.window =event.xclient.data.l[0];
               m.message_type=XdndStatus;
               m.format   =32;
               m.data.l[0]=Hwnd();
               m.data.l[1]=(xdnd_req!=NULL);
               m.data.l[2]=0;
               m.data.l[3]=0;
               m.data.l[4]=XdndActionCopy;

               XSendEvent(XDisplay, event.xclient.data.l[0], false, NoEventMask, (XEvent*)&m);
               XFlush(XDisplay);

               XWindow child=NULL;
               int x=(event.xclient.data.l[2]>>16)&0xFFFF,
                   y= event.xclient.data.l[2]     &0xFFFF;
               XTranslateCoordinates(XDisplay, DefaultRootWindow(XDisplay), Hwnd(), x, y, &xdnd_pos.x, &xdnd_pos.y, &child);
            }else
            if(event.xclient.message_type==XdndDrop)
            {
               if(!xdnd_req)
               {
                  XClientMessageEvent m; Zero(m);
                  m.type        =ClientMessage;
                  m.display     =event.xclient.display;
                  m.window      =event.xclient.data.l[0];
                  m.message_type=XdndFinished;
                  m.format      =32;
                  m.data.l[0]   =Hwnd();
                  m.data.l[1]   =0;
                  m.data.l[2]   =0;
                  XSendEvent(XDisplay, event.xclient.data.l[0], false, NoEventMask, (XEvent*)&m);
               }else
               {
                  if(xdnd_version>=1)
                  {
                     XConvertSelection(XDisplay, XdndSelection, xdnd_req, PRIMARY, Hwnd(), event.xclient.data.l[2]);
                  }else
                  {
                     XConvertSelection(XDisplay, XdndSelection, xdnd_req, PRIMARY, Hwnd(), CurrentTime);
                  }
               }
            }
         }break;

         case SelectionNotify:
         {
            if(event.xselection.target==xdnd_req)
            {
               Memc<Str> names;
               XProp p; ReadProperty(p, Hwnd(), PRIMARY);
               if(p.format==8)
               {
                  Str8 s=Replace((CChar8*)p.data, "file://", S), o;
                  o.reserve(s.length());
                  FREPA(s)
                  {
                     Char8 c=s[i];
                     if(c!='\r')
                     {
                        if(c=='%')
                        {
                           o+=Char8((CharInt(s[i+1])<<4)|CharInt(s[i+2]));
                           i+=2;
                        }else o+=c;
                     }
                  }
                  Split(names, FromUTF8(o), '\n');
                  if(names.elms() && !names.last().is())names.removeLast();
               }
               XFree(p.data);

               // send reply
               XClientMessageEvent m; Zero(m);
               m.type        =ClientMessage;
               m.display     =XDisplay;
               m.window      =xdnd_source;
               m.message_type=XdndFinished;
               m.format      =32;
               m.data.l[0]=Hwnd();
               m.data.l[1]=1;
               m.data.l[2]=XdndActionCopy;
               XSendEvent(XDisplay, xdnd_source, false, NoEventMask, (XEvent*)&m);
               XSync(XDisplay, false);

               if(drop)
               {
                  Vec2 pos=D.windowPixelToScreen(xdnd_pos);
                  drop(names, Gui.objAtPos(pos), pos);
               }
            }
         }break;

         case SelectionRequest:
         {
            int            format=0;
            unsigned long  items=0;
            unsigned long  overflow=0;
            unsigned char *data=null;
            XSelectionRequestEvent &req=event.xselectionrequest;
            XEvent sevent; Zero(sevent);
            sevent.xany.type           =SelectionNotify;
            sevent.xselection.selection=req.selection;
            sevent.xselection.target   =NULL;
            sevent.xselection.property =NULL;
            sevent.xselection.requestor=req.requestor;
            sevent.xselection.time     =req.time;
            if(!XGetWindowProperty(XDisplay, DefaultRootWindow(XDisplay), XA_CUT_BUFFER0, 0, INT_MAX/4, false, req.target, &sevent.xselection.target, &format, &items, &overflow, &data))
            {
               Atom XA_TARGETS=XInternAtom(XDisplay, "TARGETS", 0);
               if(sevent.xselection.target==req.target)
               {
                  XChangeProperty(XDisplay, req.requestor, req.property, sevent.xselection.target, format, PropModeReplace, data, items);
                  sevent.xselection.property=req.property;
               }else
               if(XA_TARGETS==req.target)
               {
                  Atom SupportedFormats[]={sevent.xselection.target, XA_TARGETS};
                  XChangeProperty(XDisplay, req.requestor, req.property, XA_ATOM, 32, PropModeReplace, (unsigned char*)SupportedFormats, Elms(SupportedFormats));
                  sevent.xselection.property=req.property;
               }
            }
            if(data)XFree(data);
            XSendEvent(XDisplay, req.requestor, false, 0, &sevent);
            XSync(XDisplay, false);
         }break;
      }
      switch(event.type)
      {
         /*case KeymapNotify: if(active()) ignore this because of the issue 1) Mouse-click on App to activate (must be by mouse) 2) Alt+Tab to lose focus 3) Alt+Tab to restore focus 4) Alt gets stuck, and worse if Alt+F4 some other app then Alt+F4 gets copied to this app as well, if wanted to be restored then 'KeymapStateMask' have to be re-enabled
         {
            REPA(event.xkeymap.key_vector)if(Byte b=(Byte)event.xkeymap.key_vector[i])
               REPD(bit, 8)if(b&(1<<bit))
            {
               Int index=(bit+(i<<3));
               if(InRange(index, ScanCodeToKey))Kb.push(ScanCodeToKey[index]);
            }
         }break;*/

         case MappingNotify:
         {
            XMappingEvent &map=event.xmapping;
            if(map.request==MappingKeyboard)Kb.setLayout();
         }break;
         
         case GenericEvent: if(event.xcookie.extension==XInput2Extension)
         {
            if(XGetEventData(XDisplay, &event.xcookie))
            {
               switch(event.xcookie.evtype)
               {
                  case XI_RawMotion:
                  {
                     XIRawEvent &raw=*(XIRawEvent*)event.xcookie.data;
                     Dbl delta[2]; SetXInputValues(raw.raw_values, raw.valuators.mask, raw.valuators.mask_len, delta, Elms(delta));
                     Ms._delta_relative.x+=delta[0];
                     Ms._delta_relative.y-=delta[1];
                  }break;

                  case XI_RawButtonPress: if(active()) // this will get called even if app is not active
                  {
                     XIRawEvent &raw=*(XIRawEvent*)event.xcookie.data;
                     Int b=raw.detail-5; // 8 is button #3, 9 is button #4, .., other buttons are handled in 'ButtonPress'
                     if( b>=3 && InRange(b, ButtonPressCount)) // on Linux there's some issue that extra buttons get pushed,pushed,released in the same frame
                     {
                        if(!ButtonPressCount[b]++)Ms.push(b); // push only at first
                     }
                  }break;

                  case XI_RawButtonRelease:
                  {
                     XIRawEvent &raw=*(XIRawEvent*)event.xcookie.data;
                     Int b=raw.detail-5; // 8 is button #3, 9 is button #4, .., other buttons are handled in 'ButtonRelease'
                     if( b>=3 && InRange(b, ButtonPressCount) && ButtonPressCount[b])
                     {
                        if(!--ButtonPressCount[b])Ms.release(b); // release only at last
                     }
                  }break;

                  case XI_TouchBegin:
                  {
                     XIDeviceEvent &dev=*(XIDeviceEvent*)event.xcookie.data;
                     //xev->sourceid, xev->detail, xev->event_x, xev->event_y
                  }break;

                  case XI_TouchEnd:
                  {
                     XIDeviceEvent &dev=*(XIDeviceEvent*)event.xcookie.data;
                     //xev->sourceid, xev->detail, xev->event_x, xev->event_y
                  }break;

                  case XI_TouchUpdate:
                  {
                     XIDeviceEvent &dev=*(XIDeviceEvent*)event.xcookie.data;
                     //xev->sourceid, xev->detail, xev->event_x, xev->event_y
                  }break;
               }
               XFreeEventData(XDisplay, &event.xcookie);
            }
         }break;
      }
   }
   if(Deactivate && !--Deactivate){setActive(false); XUnsetICFocus(IC);}
#endif

   if(!active() && !_close)
      if(Int wait=((flag&APP_WORK_IN_BACKGROUND) ? background_wait : -1))
   {
      if(wait<0) // unlimited wait
      {
      #if WINDOWS_OLD
         WaitMessage(); goto start;
      #elif WINDOWS_NEW
         Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher->ProcessEvents(Windows::UI::Core::CoreProcessEventsOption::ProcessOneAndAllPending);
         goto again;
      #elif MAC
         if([NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantFuture] inMode:NSDefaultRunLoopMode dequeue:NO])goto start; // 'distantFuture' will wait for an event
        _close=true; // if no event got received in unlimited wait, then perhaps app wants to be closed, request the main thread to close the app (but don't call 'App.close' because that would call quit)
      #elif LINUX
         goto process; // jump to 'XNextEvent' which will block until event occurs
      #endif
      }else // finite wait
      {
      #if MAC
         if(!untilDate)untilDate=[NSDate dateWithTimeIntervalSinceNow:(wait/1000.0)];
         if([NSApp nextEventMatchingMask:NSEventMaskAny untilDate:untilDate inMode:NSDefaultRunLoopMode dequeue:NO])goto start;
      #else
            if(wait_end_set) // if we already have the end time limit
            {
               wait=wait_end-Time.curTimeMs(); if(wait<=0)goto stop; // calculate remaining time
            }else
            {
               wait_end=Time.curTimeMs()+wait; wait_end_set=true;
            }
         #if WINDOWS_OLD
            if(MsgWaitForMultipleObjects(0, null, false, wait, QS_ALLINPUT)!=WAIT_TIMEOUT)goto start;
         #else
            Time.wait(1); goto start;
         #endif
      #endif
      }
   }
stop:

   if(_window_resized.x>0 && _window_resized.y>0)
   {
      D.modeSet(_window_resized.x, _window_resized.y);
     _window_resized=-1;
   }
#if MAC
   PreventResizing=false;
#endif
}
#if WINDOWS || MAC || LINUX
void Application::loop()
{
   for(;;)
   {
      windowMsg();
      if(_close)break;

   #if WINDOWS_OLD
      Updating=true; // set as updating to prevent WM_TIMER-based updates occurring inside this scope
   #endif
      update();
   #if WINDOWS_OLD
      Updating=false;
   #endif
   }
}
#endif
/******************************************************************************/
void InitWindow() // this is called again inside 'App.coInitialize' !!
{
#if WINDOWS_OLD
   CoCreateInstance(CLSID_TaskbarList, null, CLSCTX_ALL, IID_ITaskbarList3, (Ptr*)&TaskbarList);
   SetLastError(0); // clear error 2
#elif LINUX
   if(XDisplay)
   {
      FIND_ATOM(     WM_STATE);
      FIND_ATOM(_NET_WM_STATE);
      FIND_ATOM(_NET_WM_STATE_HIDDEN);
      FIND_ATOM(_NET_WM_STATE_FOCUSED);
      FIND_ATOM(_NET_WM_STATE_MAXIMIZED_HORZ);
      FIND_ATOM(_NET_WM_STATE_MAXIMIZED_VERT);
      FIND_ATOM(_NET_WM_STATE_FULLSCREEN);
      FIND_ATOM(_NET_WM_STATE_DEMANDS_ATTENTION);
      FIND_ATOM(_NET_FRAME_EXTENTS);
      FIND_ATOM(_NET_WM_NAME);
      FIND_ATOM(UTF8_STRING);
      FIND_ATOM(_MOTIF_WM_HINTS);
   }
#endif
}
void ShutWindow() // this is called again inside 'App.coInitialize' !!
{
#if WINDOWS_OLD
   RELEASE(TaskbarList);
#endif
}
/******************************************************************************/
}
/******************************************************************************/

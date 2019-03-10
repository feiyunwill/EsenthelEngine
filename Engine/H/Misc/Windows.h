/******************************************************************************

   Use 'WindowCapture' to capture visual contents of a system window.

   Use 'Window' functions to handle OS window management.

/******************************************************************************/
struct WindowCapture // System Window Image Capture
{
   void del    (                           ); // delete manually
   Bool capture(Image &image, Ptr hwnd=null); // capture 'hwnd' window client screen to 'image' (use null for 'hwnd' to capture full desktop), false on fail

  ~WindowCapture() {del();}
   WindowCapture() {data=null;}

private:
   Ptr data;
   NO_COPY_CONSTRUCTOR(WindowCapture);
};
/******************************************************************************/
void  WindowSetText    (C Str &text     ,            Ptr hwnd=App.hwnd()); // set window text title
Str   WindowGetText    (                             Ptr hwnd=App.hwnd()); // get window text title
void  WindowMinimize   (Bool force=false,            Ptr hwnd=App.hwnd()); // minimize window
void  WindowMaximize   (Bool force=false,            Ptr hwnd=App.hwnd()); // maximize window
void  WindowReset      (Bool force=false,            Ptr hwnd=App.hwnd()); // reset    window from    maximized/minimized to normal state (maximized/minimized -> normal)
void  WindowToggle     (Bool force=false,            Ptr hwnd=App.hwnd()); // toggle   window between maximized          and normal state (maximized          <-> normal)
void  WindowActivate   (                             Ptr hwnd=App.hwnd()); // activate window
void  WindowHide       (                             Ptr hwnd=App.hwnd()); // hide     window
void  WindowShow       (                             Ptr hwnd=App.hwnd()); // show     window
void  WindowClose      (                             Ptr hwnd=App.hwnd()); // close    window
void  WindowFlash      (                             Ptr hwnd=App.hwnd()); // flash    window
void  WindowSetNormal  (                             Ptr hwnd=App.hwnd()); // set      window taskbar to be displayed as normal                             (this will work only on Window 7 or newer)
void  WindowSetWorking (                             Ptr hwnd=App.hwnd()); // set      window taskbar to be displayed as working with unknown progress      (this will work only on Window 7 or newer)
void  WindowSetProgress(Flt progress,                Ptr hwnd=App.hwnd()); // set      window taskbar to be displayed as working with 'progress' 0..1 value (this will work only on Window 7 or newer)
void  WindowSetPaused  (Flt progress,                Ptr hwnd=App.hwnd()); // set      window taskbar to be displayed as paused  with 'progress' 0..1 value (this will work only on Window 7 or newer)
void  WindowSetError   (Flt progress,                Ptr hwnd=App.hwnd()); // set      window taskbar to be displayed as error   with 'progress' 0..1 value (this will work only on Window 7 or newer)
Byte  WindowGetAlpha   (                             Ptr hwnd=App.hwnd()); // get      window opacity (0=transparent, 255=opaque)
void  WindowAlpha      (Byte alpha  ,                Ptr hwnd=App.hwnd()); // set      window opacity (0=transparent, 255=opaque)
void  WindowMove       (Int dx, Int dy,              Ptr hwnd=App.hwnd()); // move     window by delta
void  WindowPos        (Int  x, Int  y,              Ptr hwnd=App.hwnd()); // set      window position
void  WindowSize       (Int  w, Int  h, Bool client, Ptr hwnd=App.hwnd()); // set      window size
VecI2 WindowSize       (                Bool client, Ptr hwnd=App.hwnd()); // get      window size     , 'client'=if take only the client size      (not including the borders)
RectI WindowRect       (                Bool client, Ptr hwnd=App.hwnd()); // get      window rectangle, 'client'=if take only the client rectangle (not including the borders)
Bool  WindowMaximized  (                             Ptr hwnd=App.hwnd()); // if  window is maximized
Bool  WindowMinimized  (                             Ptr hwnd=App.hwnd()); // if  window is minimized
Ptr   WindowActive     (                                                ); // get active window
Ptr   WindowMouse      (                                                ); // get window under mouse cursor
Ptr   WindowParent     (                             Ptr hwnd           ); // get          parent of window
Ptr   WindowParentTop  (                             Ptr hwnd           ); // get most top parent of window
void  WindowSendData   (CPtr data, Int size,         Ptr hwnd           ); // send binary data to an application of the specified window, that application can receive the data using "App.receive_data" callback function
UInt  WindowProc       (                             Ptr hwnd           ); // get process ID of window
void  WindowList       (MemPtr<Ptr> hwnds                               ); // get list of all window handles in the System

void WindowMsgBox(C Str &title, C Str &text, Bool error=false); // show OS message box, 'error'=if display as error or message

#if EE_PRIVATE
void InitWindow();
void ShutWindow();
#endif
/******************************************************************************/

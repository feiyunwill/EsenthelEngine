/******************************************************************************/
#include "stdafx.h"
#include "MyWindow.h"
#include "MyApplication.h"
/******************************************************************************/
namespace EE
{
Bool PreventResizing;
}
/******************************************************************************/
// WINDOW DELEGATE
/******************************************************************************/
@interface MyWindowDelegate : NSObject<NSWindowDelegate>
{
}
@end

@implementation MyWindowDelegate
-(NSSize)windowWillResize:(NSWindow*)window toSize:(NSSize)newSize
{
   if(!FlagTest(App.flag, APP_RESIZABLE) && [[NSApp currentEvent] type]==NSLeftMouseDragged)PreventResizing=true; // in case the app OS Window has resizable controls, but we don't want to support them, then we need to prevent resizing until dragging has finished
   return PreventResizing ? [window frame].size : newSize;
}
-(BOOL)windowShouldClose:(id)sender
{
   if(!(App.flag&APP_NO_CLOSE))App.close();
   return NO;
}
-(void)windowDidMiniaturize  :(NSNotification*)notification {App._minimized=true ; SetActive();}
-(void)windowDidDeminiaturize:(NSNotification*)notification {App._minimized=false; SetActive();}
@end
/******************************************************************************/
// WINDOW
/******************************************************************************/
@implementation MyWindow
-(void)dealloc
{
   [self.delegate release]; self.delegate=null;
   [super dealloc];
}
-(id)initWithContentRect:(NSRect)contentRect
               styleMask:(NSUInteger)windowStyle
                 backing:(NSBackingStoreType)bufferingType
                   defer:(BOOL)deferCreation
{
   // FlagSet(windowStyle, (UInt)NSResizableWindowMask, FlagTest(App.flag, APP_RESIZABLE|APP_MAXIMIZABLE));  don't use because App is not set yet
   self=[super initWithContentRect:contentRect styleMask:windowStyle backing:bufferingType defer:deferCreation];
   self.collectionBehavior|=NSWindowCollectionBehaviorFullScreenNone; // this will set window maximize button to maximize instead of entering fullscreen
   MyWindowDelegate *window_delegate=[[MyWindowDelegate alloc] init]; [self setDelegate:window_delegate]; //[window_delegate release]; we can't release the delegate here, because it will stop working, instead we need to release it in 'dealloc'
   return self;
}
@end
/******************************************************************************/

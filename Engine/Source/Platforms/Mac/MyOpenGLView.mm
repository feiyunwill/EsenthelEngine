/******************************************************************************/
#include "stdafx.h"
#include "MyOpenGLView.h"
#include "MyWindow.h"
namespace EE{
/******************************************************************************/
       MyOpenGLView *OpenGLView;
static ImageRTPtr    LiveResize;
/******************************************************************************/
}
/******************************************************************************/
@implementation MyOpenGLView
/******************************************************************************/
// DRAG AND DROP
/******************************************************************************/
-(NSDragOperation) draggingEntered:(id<NSDraggingInfo>)sender
{
   NSDragOperation drag_mask=[sender draggingSourceOperationMask];
   if(NSPasteboard *pasteboard=[sender draggingPasteboard])
   if([[pasteboard types] containsObject:NSFilenamesPboardType])
   {
      if(drag_mask&NSDragOperationLink)return NSDragOperationLink;
      if(drag_mask&NSDragOperationCopy)return NSDragOperationCopy;
   }
   return NSDragOperationNone;
}
-(BOOL) performDragOperation:(id<NSDraggingInfo>)sender
{
   if(App.drop)
   if(NSPasteboard *pasteboard=[sender draggingPasteboard])
   if([[pasteboard types] containsObject:NSFilenamesPboardType])
   if(NSArray *files=[pasteboard propertyListForType:NSFilenamesPboardType])
   {
      Memc<Str> names; FREP([files count])if(NSString *string=[files objectAtIndex: i])names.add(string);
      if(names.elms())App.drop(names, Gui.objAtPos(Ms.pos()), Ms.pos()); // don't use 'Gui.msLit' but 'Gui.objAtPos' because this is Apple callback and 'Ms.pos' may be newer than of 'Gui.msLit'
   }
   return YES;
}
/******************************************************************************/
// INPUT
/******************************************************************************/
-(void) scrollWheel   :(NSEvent*)theEvent {Ms._wheel.x-=[theEvent deltaX]; Ms._wheel.y+=[theEvent deltaY];} // deltaY is the normal mouse wheel, deltaX is the horizontal wheel (or vertical with Shift pressed)
-(void) mouseDown     :(NSEvent*)theEvent {Ms. push   ([theEvent buttonNumber]);}
-(void) mouseUp       :(NSEvent*)theEvent {Ms. release([theEvent buttonNumber]);}
-(void) rightMouseDown:(NSEvent*)theEvent {[self mouseDown:theEvent];}
-(void) otherMouseDown:(NSEvent*)theEvent {[self mouseDown:theEvent];}
-(void) rightMouseUp  :(NSEvent*)theEvent {[self mouseUp  :theEvent];}
-(void) otherMouseUp  :(NSEvent*)theEvent {[self mouseUp  :theEvent];}
-(void) mouseEntered  :(NSEvent*)theEvent {Ms._on_client=true ; Ms.resetVisibility();}
-(void) mouseExited   :(NSEvent*)theEvent {Ms._on_client=false; Ms.resetVisibility();}
/******************************************************************************/
// DRAW
/******************************************************************************/
-(void) drawRect:(NSRect)rect
{
   if(D.created())
   {
      if(App.minimized() || (D.full() && !App.active()))return;

      SyncLocker locker(D._lock);

      if(!D._resetting)
      {
         if([self inLiveResize]) // resizing
         {
            if(LiveResize)
            {
               D.viewReset();
               RectI   viewport=D._viewport;
                                D.viewport(RectI(0, 0, RoundPos(rect.size.width), RoundPos(rect.size.height))); // force full window viewport
               ALPHA_MODE alpha=D.alpha   (ALPHA_NONE); LiveResize->drawFs(FIT_FULL, FILTER_LINEAR);
                                D.alpha   (alpha     );
                                D.viewport(viewport  );
               goto ok;
            }         
         }else
         if(DrawState())return; // 'DrawState' already calls 'D.flip'
      }
      D.clearCol();
   ok:
      D.flip(); // flip screen
   }
}
/******************************************************************************/
-(void)setLayer:(CALayer*)layer
{
   [super setLayer:layer];
   [OpenGLContext update]; // this is needed on Xcode 10, which shipped on Mac OS Mojave, without this line, the screen will be black initially, and only appear after resizing window, this line fixes that
}
-(void) setFrameSize:(NSSize)newSize
{
   [super setFrameSize:newSize];
   [OpenGLContext update]; // without this, client area will be rendered incorrectly (black bars, flickering, offsets)
#if 1
   App._window_resized=WindowSize(true);
#else // can't use this because if app was started in fullscreen then it will return incorrect sizes (including the border)
   NSRect rect=[OpenGLView bounds]; App._window_resized.set(RoundPos(rect.size.width), RoundPos(rect.size.height));
#endif
}
-(void) viewWillStartLiveResize
{
   if(D.created() && !D._resetting && StateActive)
   {
      DrawState(); // do not surround 'DrawState' with '_lock' lock because it handles it on its own
      SyncLocker locker(D._lock);
      if(LiveResize.find(ImageRTDesc(Renderer._main.w(), Renderer._main.h(), IMAGERT_RGB)))
      {
         Renderer._main.copyHw(*LiveResize, true); // doesn't use Alpha
      #if GL
         glFlush(); // without this 'LiveResize' will be empty or look corrupted
      #endif
      }
   }
   [super viewWillStartLiveResize];
}
-(void) viewDidEndLiveResize
{
   LiveResize.clear();
   [super viewDidEndLiveResize];
#if 1
   VecI2 size=WindowSize(true);
#else // can't use this because if app was started in fullscreen then it will return incorrect sizes (including the border)
   NSRect rect=[self bounds]; VecI2 size(RoundPos(rect.size.width), RoundPos(rect.size.height));
#endif
   D.modeSet(size.x, size.y); // reset here already because draw can be requested before we detect size change in update
}
/******************************************************************************/
// CALLBACKS
/******************************************************************************/
-(BOOL)acceptsFirstResponder {return YES;}
-(BOOL)becomeFirstResponder  {return YES;}
-(BOOL)resignFirstResponder  {return YES;}
/******************************************************************************/
@end
/******************************************************************************/

/******************************************************************************/
#include "stdafx.h"
#include "MyApplication.h"
namespace EE{
/******************************************************************************/
struct KeyState
{
   KB_KEY b ;
   Bool   on;
};
/******************************************************************************/
extern VecI2 MouseIgnore;
static Bool  Active;   void SetActive() {App.setActive(Active && !App.minimized());}
/******************************************************************************/
}
/******************************************************************************/
// APP DELEGATE
/******************************************************************************/
@implementation MyAppDelegate
-(void)applicationDidBecomeActive:(NSNotification*)notification {Active=true ; SetActive();}
-(void)applicationDidResignActive:(NSNotification*)notification {Active=false; SetActive();}
@end
/******************************************************************************/
// APP
/******************************************************************************/
@implementation MyApplication
- (void)dealloc
{
   [self.delegate release]; [self setDelegate:nil];
   [super dealloc];
}
-(void)sendEvent:(NSEvent*)theEvent // key input must be processed here to: capture KB_INS (insert/help key) as it is eaten by default NSWindow class and not processed further, and to capture "key up" message when using ctrl/cmd+key (as it is normally eaten by menu/toolbar)
{
   switch([theEvent type])
   {
      case NSKeyDown:
      {
         UInt code=[theEvent keyCode];
         if(InRange(code, ScanCodeToKey))
         {
            KB_KEY key=ScanCodeToKey[code];
            Kb.push(key, code);
            // !! queue characters after push !!
            if(Kb.anyCtrl()) // if Control is pressed then no characters will be received, we must add them basing on 'key'
            {
               if(key>='A' && key<='Z')Kb.queue(Char(key + (Kb.anyShift() ? 0 : 'a'-'A')), code);else
               if(key>='0' && key<='9')Kb.queue(Char(key                                ), code);
            }
         }
         // !! queue characters after push !!
         if(NSString *characters=[theEvent characters])FREP([characters length])
         {
            UInt c=[characters characterAtIndex:i];
            if(  c>=32 && c<0xF700 && c!=127 || c>0xF8FF) // range reserved by Apple for functional keys (not character keys) 127=DEL
            {
               if(Kb.anyWin() && Kb.anyShift())c=CaseUp(Char(c)); // if Win+Shift are pressed, then pressing characters will not generate them upper case, so do this manually
               Kb.queue(Char(c), code);
            }
         }
      }return;

      case NSKeyUp:
      {
         UInt code=[theEvent keyCode];
         if(InRange(code, ScanCodeToKey))Kb.release(ScanCodeToKey[code]);
      }return;

      case NSFlagsChanged:
      {
         UInt f=[theEvent modifierFlags];
         KeyState ks[]=
         {
            {KB_LCTRL , FlagTest(f, Kb.swappedCtrlCmd() ? 0x00008 : 0x00001)},
            {KB_RCTRL , FlagTest(f, Kb.swappedCtrlCmd() ? 0x00010 : 0x02000)},
            {KB_LSHIFT, FlagTest(f,                                 0x00002)},
            {KB_RSHIFT, FlagTest(f,                                 0x00004)},
            {KB_LALT  , FlagTest(f,                                 0x00020)},
            {KB_RALT  , FlagTest(f,                                 0x00040)},
            {KB_LWIN  , FlagTest(f, Kb.swappedCtrlCmd() ? 0x00001 : 0x00008)},
            {KB_RWIN  , FlagTest(f, Kb.swappedCtrlCmd() ? 0x02000 : 0x00010)},
            {KB_CAPS  , FlagTest(f,                                 0x10000)}, // enables access to only whether CapsLock is on or off (not if pushed/released)
         };
         REPA(ks)if(Kb.b(ks[i].b)!=ks[i].on){if(ks[i].on)Kb.push(ks[i].b, -1);else Kb.release(ks[i].b);}
      }return;

   	/*case NSAppKitDefined:
      {
      	switch([theEvent subtype])
         {
         	case NSApplicationActivatedEventType:
            {
            }break;

            case NSApplicationDeactivatedEventType:
            {
            }break;
         }
      }break;*/
         
      case NSMouseMoved:
      case NSLeftMouseDragged:
      case NSRightMouseDragged:
      case NSOtherMouseDragged:
      {
         Ms._delta_relative.x+=[theEvent deltaX]-MouseIgnore.x; MouseIgnore.x=0;
         Ms._delta_relative.y-=[theEvent deltaY]-MouseIgnore.y; MouseIgnore.y=0;
      }return;
   }
   [super sendEvent:theEvent];
}
@end
/******************************************************************************/

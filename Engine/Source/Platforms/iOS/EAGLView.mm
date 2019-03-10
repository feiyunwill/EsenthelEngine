/******************************************************************************/
#include "iOS.h"
/******************************************************************************/
Bool DontRemoveThisOrEAGLViewClassWontBeLinked;
/******************************************************************************/
namespace EE{
/******************************************************************************/
void (*ResizeAdPtr)();

static const Bool iOS_9_0=(Compare(OSVerNumber(), VecI4(9, 0, 0, 0))>=0); // if iOS 9.0 available

static KB_KEY KeyMap[128];
static inline void SetKeyMap(Char c, KB_KEY key) {U16 u=c; if(InRange(u, KeyMap))KeyMap[u]=key;}
static void InitKeyMap()
{
   SetKeyMap('a', KB_A);
   SetKeyMap('A', KB_A);
   SetKeyMap('b', KB_B);
   SetKeyMap('B', KB_B);
   SetKeyMap('c', KB_C);
   SetKeyMap('C', KB_C);
   SetKeyMap('d', KB_D);
   SetKeyMap('D', KB_D);
   SetKeyMap('e', KB_E);
   SetKeyMap('E', KB_E);
   SetKeyMap('f', KB_F);
   SetKeyMap('F', KB_F);
   SetKeyMap('g', KB_G);
   SetKeyMap('G', KB_G);
   SetKeyMap('h', KB_H);
   SetKeyMap('H', KB_H);
   SetKeyMap('i', KB_I);
   SetKeyMap('I', KB_I);
   SetKeyMap('j', KB_J);
   SetKeyMap('J', KB_J);
   SetKeyMap('k', KB_K);
   SetKeyMap('K', KB_K);
   SetKeyMap('l', KB_L);
   SetKeyMap('L', KB_L);
   SetKeyMap('m', KB_M);
   SetKeyMap('M', KB_M);
   SetKeyMap('n', KB_N);
   SetKeyMap('N', KB_N);
   SetKeyMap('o', KB_O);
   SetKeyMap('O', KB_O);
   SetKeyMap('p', KB_P);
   SetKeyMap('P', KB_P);
   SetKeyMap('q', KB_Q);
   SetKeyMap('Q', KB_Q);
   SetKeyMap('r', KB_R);
   SetKeyMap('R', KB_R);
   SetKeyMap('s', KB_S);
   SetKeyMap('S', KB_S);
   SetKeyMap('t', KB_T);
   SetKeyMap('T', KB_T);
   SetKeyMap('u', KB_U);
   SetKeyMap('U', KB_U);
   SetKeyMap('v', KB_V);
   SetKeyMap('V', KB_V);
   SetKeyMap('w', KB_W);
   SetKeyMap('W', KB_W);
   SetKeyMap('x', KB_X);
   SetKeyMap('X', KB_X);
   SetKeyMap('y', KB_Y);
   SetKeyMap('Y', KB_Y);
   SetKeyMap('z', KB_Z);
   SetKeyMap('Z', KB_Z);

   SetKeyMap('1', KB_1);
   SetKeyMap('!', KB_1);
   SetKeyMap('2', KB_2);
   SetKeyMap('@', KB_2);
   SetKeyMap('3', KB_3);
   SetKeyMap('#', KB_3);
   SetKeyMap('4', KB_4);
   SetKeyMap('$', KB_4);
   SetKeyMap('5', KB_5);
   SetKeyMap('%', KB_5);
   SetKeyMap('6', KB_6);
   SetKeyMap('^', KB_6);
   SetKeyMap('7', KB_7);
   SetKeyMap('&', KB_7);
   SetKeyMap('8', KB_8);
   SetKeyMap('*', KB_8);
   SetKeyMap('9', KB_9);
   SetKeyMap('(', KB_9);
   SetKeyMap('0', KB_0);
   SetKeyMap(')', KB_0);

   SetKeyMap('-' , KB_SUB       );
   SetKeyMap('_' , KB_SUB       );
   SetKeyMap('=' , KB_EQUAL     );
   SetKeyMap('+' , KB_EQUAL     );
   SetKeyMap('[' , KB_LBRACKET  );
   SetKeyMap('{' , KB_LBRACKET  );
   SetKeyMap(']' , KB_RBRACKET  );
   SetKeyMap('}' , KB_RBRACKET  );
   SetKeyMap(';' , KB_SEMICOLON );
   SetKeyMap(':' , KB_SEMICOLON );
   SetKeyMap('\'', KB_APOSTROPHE);
   SetKeyMap('"' , KB_APOSTROPHE);
   SetKeyMap(',' , KB_COMMA     );
   SetKeyMap('<' , KB_COMMA     );
   SetKeyMap('.' , KB_DOT       );
   SetKeyMap('>' , KB_DOT       );
   SetKeyMap('/' , KB_SLASH     );
   SetKeyMap('?' , KB_SLASH     );
   SetKeyMap('\\', KB_BACKSLASH );
   SetKeyMap('|' , KB_BACKSLASH );
   SetKeyMap('`' , KB_TILDE     );
   SetKeyMap('~' , KB_TILDE     );

   SetKeyMap('\n', KB_ENTER);
   SetKeyMap('\t', KB_TAB  );
}
/******************************************************************************/
EAGLView* GetUIView()
{
   return ViewController ? (EAGLView*)ViewController.view : null;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
@implementation EAGLView
/******************************************************************************/
+(Class)layerClass
{
   return [CAEAGLLayer class];
}
/******************************************************************************/
-(id)initWithCoder:(NSCoder*)coder
{    
   if(self=[super initWithCoder:coder])
   {
      // Get the layer
      CAEAGLLayer *layer=(CAEAGLLayer*)self.layer;
      layer.opaque=true;
      layer.drawableProperties=[NSDictionary dictionaryWithObjectsAndKeys:
         [NSNumber numberWithBool:false], kEAGLDrawablePropertyRetainedBacking, // discard framebuffer contents after drawing frame, for better performance
         kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, // use RGBA8 format
         nil];

      initialized=false;
      force_touch=false;
      keyboard_visible=false;
      display_link=nil;
      InitKeyMap();

      self.multipleTouchEnabled=true;
      self.contentScaleFactor=ScreenScale;

      // setup notifications for the keyboard
      [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWasShown    :) name:UIKeyboardDidShowNotification  object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillBeHidden:) name:UIKeyboardWillHideNotification object:nil]; 
   }
   return self;
}
/******************************************************************************/
-(void)keyboardWasShown:(NSNotification*)aNotification
{
   Kb._visible=true;
   NSDictionary *info=[aNotification userInfo];
   CGRect rect=[[info objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
   RectI  recti(Round(rect.origin.x*ScreenScale), Round(rect.origin.y*ScreenScale), Round((rect.origin.x+rect.size.width)*ScreenScale), Round((rect.origin.y+rect.size.height)*ScreenScale));
   switch([UIApplication sharedApplication].statusBarOrientation) // don't use 'App.orientation' as it's one frame ahead/behind than this call
   {
      default                                      : Kb._recti=recti; break;
      case UIInterfaceOrientationPortraitUpsideDown: Kb._recti.set(recti.min.x, D.resH()-recti.max.y, recti.max.x, D.resH()-recti.min.y); break;
      case UIInterfaceOrientationLandscapeLeft     : Kb._recti.set(recti.min.y,          recti.min.x, recti.max.y,          recti.max.x); break;
      case UIInterfaceOrientationLandscapeRight    : Kb._recti.set(recti.min.y, D.resH()-recti.max.x, recti.max.y, D.resH()-recti.min.x); break;
   }
}
-(void)keyboardWillBeHidden:(NSNotification*)aNotification {Kb._visible=false;}
/******************************************************************************/
-(void)detectForceTouch
{
   force_touch=(iOS_9_0 && [[self traitCollection] forceTouchCapability]==UIForceTouchCapabilityAvailable); // API available on iOS 9.0+ ONLY
}
-(void)traitCollectionDidChange:(UITraitCollection *)previousTraitCollection
{
   [super traitCollectionDidChange:previousTraitCollection];
   [self detectForceTouch];
}
/******************************************************************************/
-(void)layoutSubviews // this is called when the layer is initialized, resized or a sub view is added (like ads)
{
   if(App._closed)return; // do nothing if app called 'Exit'
   // always proceed, because issues may appear when screen is rotated while full screen ad is displayed, and then we go back to the application
   if(!initialized)
   {
      initialized=true;
      [self detectForceTouch];
      if(!App.create())Exit("Failed to initialize the application"); // something failed
      [self setUpdate];
   }else
   {
      D._res.zero(); D.modeSet(1, 1); // clear current res to force setting mode according to newly detected resolution, for now use dummy values, actual values will be detected later
   }
   if(ResizeAdPtr)ResizeAdPtr(); // re-position banner after having everything, call this from a pointer to function (and not directly to the advertisement codes) to avoid force linking to advertisement (which then links to google admob lib, which uses 'advertisingIdentifier' which is prohibited when ads are not displayed in iOS, also linking to admob increases the app binary size)
}
/******************************************************************************/
-(void)update:(id)sender
{
   if(App._close)
   {
      App.del(); // manually call shut down
      ExitNow(); // force exit as iOS does not offer a clean way to do it
   }
   if(App.active())
   {
      Bool kb_visible =Kb.visibleWanted();
      if(  kb_visible!=keyboard_visible || Kb._refresh_visible){if(keyboard_visible=kb_visible)[self becomeFirstResponder];else [self resignFirstResponder]; Kb._refresh_visible=false;}
      App.update();
   }
}
/******************************************************************************/
-(void)setUpdate
{
   SyncLocker locker(D._lock); // thread safety because this method can be called by 'Exit'
   bool run =(initialized && App.active() && !App._closed); // don't run if app called 'Exit'
   if(  run!=(display_link!=null))
   {
      if(run)
      {
         display_link=[CADisplayLink displayLinkWithTarget:self selector:@selector(update:)];
       //display_link.preferredFramesPerSecond=..; this is not needed because default value is 0, which is max possible frame rate - https://developer.apple.com/documentation/quartzcore/cadisplaylink/1648421-preferredframespersecond?language=objc
         [display_link addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
      }else
      {
         [display_link invalidate];
         display_link=nil;
      }
   }
}
/******************************************************************************/
-(void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
	for(UITouch *touch in touches)
   {
      CGPoint pos=[touch locationInView:self];
      Vec2    p(pos.x, pos.y); p*=ScreenScale;
      VecI2   pi=Round(p); p=D.windowPixelToScreen(p);
      Touch  *t=FindTouchByHandle(touch); // find existing one
      if(!t) // create new one
      {
         t=&Touches.New().init(pi, p, touch, iOS_9_0 ? touch.type==UITouchTypeStylus : false); // API available on iOS 9.0+ ONLY
      }else
      {
         t->_deltai+=pi-t->_posi;
         t->_posi   =pi;
         t->_pos    =p;
      }
      Int taps =[touch tapCount]; // it starts from 1
      t->_first=(taps&1); // it's a first click on odd numbers, 1, 3, 5, ..
      t->_state=BS_ON|BS_PUSHED;
      if(!t->_first)t->_state|=BS_DOUBLE; // set double clicking only on even numbers, 2, 4, 6, ..
      t->_force=(force_touch ? touch.force : 1); // API available for 'force_touch' ONLY
   }
}
-(void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event
{
   for(UITouch *touch in touches)
   {
      CGPoint pos=[touch locationInView:self];
      Vec2    p(pos.x, pos.y); p*=ScreenScale;
      VecI2   pi=Round(p); p=D.windowPixelToScreen(p);
      Touch  *t=FindTouchByHandle(touch); // find existing one
      if(!t) // create new one
      {
         t=&Touches.New().init(pi, p, touch, iOS_9_0 ? touch.type==UITouchTypeStylus : false); // API available on iOS 9.0+ ONLY
         t->_state=BS_ON|BS_PUSHED;
         t->_force=1;
      }
      t->_deltai+=pi-t->_posi;
      t->_posi   =pi;
      t->_pos    =p;
      if(force_touch)t->_force=touch.force; // API available for 'force_touch' ONLY
   }
}
-(void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
   for(UITouch *touch in touches)if(Touch *t=FindTouchByHandle(touch))
   {
      CGPoint pos=[touch locationInView:self];
      Vec2    p(pos.x, pos.y); p*=ScreenScale;
      VecI2   pi=Round(p); p=D.windowPixelToScreen(p);
      t->_deltai+=pi-t->_posi;
      t->_posi   =pi;
      t->_pos    =p;
      t->_remove =true;
      if(t->_state&BS_ON) // check for state in case it was manually eaten
      {
         t->_state|= BS_RELEASED;
         t->_state&=~BS_ON;
      }
   }
}
-(void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event
{
   [self touchesEnded:touches withEvent:event];
}
/******************************************************************************/
-(void)insertText:(NSString*)text
{
   Str s=text; FREPA(s)
   {
      Char c=s[i]; Kb.queue(c, -1);
    //U16  u=c   ; if(InRange(u, KeyMap)){KB_KEY k=KeyMap[u]; Kb.push(k); Kb.release(k);}
   }
}
-(void)deleteBackward
{
   Kb.push   (KB_BACK, -1);
   Kb.release(KB_BACK);
}
-(BOOL)hasText                 {return true;}
-(BOOL)canBecomeFirstResponder {return true;}
/******************************************************************************/
@end
/******************************************************************************/

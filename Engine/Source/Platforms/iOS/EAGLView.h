/******************************************************************************/
@interface EAGLView : UIView<UIKeyInput>
{    
   BOOL           initialized, keyboard_visible, force_touch;
   CADisplayLink *display_link;
}

-(void)detectForceTouch;
-(void)update:(id)sender;
-(void)setUpdate;

@end
/******************************************************************************/
namespace EE{
/******************************************************************************/
extern void (*ResizeAdPtr)();

EAGLView* GetUIView();
/******************************************************************************/
}
/******************************************************************************/

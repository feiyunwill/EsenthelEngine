/******************************************************************************/
#include "iOS.h"
/******************************************************************************/
Bool DontRemoveThisOrMyViewControllerClassWontBeLinked;
namespace EE
{
   MyViewController *ViewController;
}
/******************************************************************************/
@implementation MyViewController
/******************************************************************************/
-(NSUInteger)supportedInterfaceOrientations
{
   if(App._closed)return 0; // prevent screen rotation if app called 'Exit'
   if(NSArray *supported_orientations=[[[NSBundle mainBundle] infoDictionary] objectForKey:@"UISupportedInterfaceOrientations"])
   {
      UInt mask=0;
      REP([supported_orientations count])
      {
         Str orient=[supported_orientations objectAtIndex:i];
         if( orient=="UIInterfaceOrientationPortrait"          )mask|=UIInterfaceOrientationMaskPortrait;else
         if( orient=="UIInterfaceOrientationPortraitUpsideDown")mask|=UIInterfaceOrientationMaskPortraitUpsideDown;else
         if( orient=="UIInterfaceOrientationLandscapeLeft"     )mask|=UIInterfaceOrientationMaskLandscapeLeft;else
         if( orient=="UIInterfaceOrientationLandscapeRight"    )mask|=UIInterfaceOrientationMaskLandscapeRight;
      }
      return mask;
   }
   return UIInterfaceOrientationMaskAll; // if not available then support all orientations
}
-(BOOL)prefersStatusBarHidden {return YES;}
/******************************************************************************/
@end
/******************************************************************************/

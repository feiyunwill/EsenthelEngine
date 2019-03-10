/******************************************************************************/
@class EAGLView;

@interface iOSAppDelegate : NSObject<UIApplicationDelegate, CLLocationManagerDelegate, FBSDKSharingDelegate>
{
   UIWindow         *window;
   UIViewController *controller;
}

@property (nonatomic, retain) IBOutlet UIWindow         *window;
@property (nonatomic, retain) IBOutlet UIViewController *controller;

@end
/******************************************************************************/
namespace EE{
/******************************************************************************/
extern void (*InitChartboostPtr)();
/******************************************************************************/
iOSAppDelegate* GetAppDelegate();
/******************************************************************************/
}
/******************************************************************************/

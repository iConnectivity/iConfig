/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICAppDelegate.h"

#import "Communicator.h"
#import "ICDeviceSelectionViewController.h"
#import "ICMIDIIndexTableViewController.h"
#import "ICMainMenuProvider.h"
#import "ICViewController.h"
#import "ICMenuViewController.h"
#import "MMDrawerController.h"
#import "MMDrawerVisualState.h"

using namespace GeneSysLib;

@implementation ICAppDelegate

- (BOOL)application:(UIApplication *)application
    didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {

#ifdef DEBUG
  server = [[ICServer alloc] init];
  [server start];
#endif

  /*
  //NSLog(@"+-------------------------------+");
  //NSLog(@"| Started iConnectivity iConfig |");
  //NSLog(@"+-------------------------------+");
   */
  self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
  // Override point for customization after application launch.

  GeneSysLib::CommPtr communicator = CommPtr(new GeneSysLib::Communicator());

  self.viewController = [[ICDeviceSelectionViewController alloc]
      initWithCommunicator:communicator];

  self.navigationController = [[UINavigationController alloc]
      initWithRootViewController:self.viewController];
  self.navigationController.navigationBarHidden = YES;

  self.navigationController.navigationBar.tintColor =
      [UIColor colorWithRed:0 green:67.0 / 255.0 blue:97.0 / 255.0 alpha:1.0];

  auto *const vc = [[ICMIDIIndexTableViewController alloc] init];
  auto *const leftMenu = [[ICMenuViewController alloc] init];

  auto *const drawerController = [[MMDrawerController alloc]
                                  initWithCenterViewController:self.navigationController
                                  leftDrawerViewController:leftMenu
                                  /*rightDrawerViewController:vc*/];
  [drawerController setMaximumLeftDrawerWidth:250.0f];
  [drawerController setMaximumRightDrawerWidth:260.0f];
  [drawerController setOpenDrawerGestureModeMask:MMOpenDrawerGestureModeBezelPanningCenterView];
  [drawerController setCloseDrawerGestureModeMask:MMOpenDrawerGestureModeAll];
  [drawerController setShouldStretchDrawer:NO];
  [drawerController setShowsStatusBarBackgroundView:NO];
  auto blockState = [MMDrawerVisualState slideVisualStateBlock];
  [drawerController setDrawerVisualStateBlock:blockState];

  /*[drawerController
   setGestureShouldRecognizeTouchBlock:^BOOL(MMDrawerController *drawerController, UIGestureRecognizer *gesture, UITouch *touch) {
     BOOL shouldRecognizeTouch = NO;
     if(drawerController.openSide == MMDrawerSideNone &&
        [gesture isKindOfClass:[UIPanGestureRecognizer class]]){
       if ([touch locationInView:drawerController.centerViewController.view].x <= 2.0)
       {
         shouldRecognizeTouch = YES;
         //NSLog(@"In all the Ifs: %@", NSStringFromClass([UISwipeGestureRecognizer class]));
       }
     }
     //NSLog(@"Out of the Ifs: %@", NSStringFromClass([gesture class]));
     //NSLog(@"%@, %d", NSStringFromCGPoint([touch locationInView:drawerController.centerViewController.view]), drawerController.openSide);
     return shouldRecognizeTouch;
   }];*/

  if ([[[UIDevice currentDevice] systemVersion] floatValue] >= 7) {
    [[UITableView appearance] setSeparatorInset:UIEdgeInsetsZero];
  }

  self.window.rootViewController = drawerController;  // self.navigationController;
  [self.window makeKeyAndVisible];

  return YES;
}

- (UINavigationController*) getNavigationController {
  return self.navigationController;
}

- (UIViewController*) getVisibleView {
  return self.navigationController.visibleViewController;
}

- (void)applicationWillResignActive:(UIApplication *)application {
  // Sent when the application is about to move from active to inactive state.
  // This can occur for certain types of temporary interruptions (such as an
  // incoming phone call or SMS message) or when the user quits the application
  // and it begins the transition to the background state.
  // Use this method to pause ongoing tasks, disable timers, and throttle down
  // OpenGL ES frame rates. Games should use this method to pause the game.
  //[browser stop];
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
  // Use this method to release shared resources, save user data, invalidate
  // timers, and store enough application state information to restore your
  // application to its current state in case it is terminated later.
  // If your application supports background execution, this method is called
  // instead of applicationWillTerminate: when the user quits.
  //[browser stop];
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
  // Called as part of the transition from the background to the inactive state;
  // here you can undo many of the changes made on entering the background.
  //[browser start];
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
  // Restart any tasks that were paused (or not yet started) while the
  // application was inactive. If the application was previously in the
  // background, optionally refresh the user interface.
  //[browser start];
}

- (void)applicationWillTerminate:(UIApplication *)application {
// Called when the application is about to terminate. Save data if
// appropriate. See also applicationDidEnterBackground:.
/*
NSLog(@"+----------------------------------+");
NSLog(@"| Terminated iConnectivity iConfig |");
NSLog(@"+----------------------------------+");
 */
#ifdef DEBUG
  [server stop];
#endif
  //[browser stop];
}

@end

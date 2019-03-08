/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>

#ifdef DEBUG
#import "ICServer.h"
#endif  // DEBUG

@interface ICAppDelegate : UIResponder<UIApplicationDelegate> {
#ifdef DEBUG
  ICServer *server;
#endif  // DEBUG
}

@property(strong, nonatomic) UIWindow *window;

@property(strong, nonatomic) UIViewController *viewController;

@property(strong, nonatomic) UINavigationController *navigationController;

@end

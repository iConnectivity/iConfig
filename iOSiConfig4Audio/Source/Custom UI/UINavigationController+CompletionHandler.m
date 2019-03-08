/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "UINavigationController+CompletionHandler.h"
#import <QuartzCore/QuartzCore.h>

@implementation UINavigationController (CompletionHandler)

- (void)completionhandler_popViewControllerAnimated:(BOOL)animated
                                  completion:(void (^)(void))completion
{
  [CATransaction begin];
  [CATransaction setCompletionBlock:completion];
  [self popViewControllerAnimated:animated];
  [CATransaction commit];
}

@end

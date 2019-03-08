/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICRunOnMain.h"

void runOnMain(void (^function)(void)) {
  assert(function);
  dispatch_async(dispatch_get_main_queue(), function);
}

void runOnMainAfter(double delayInSeconds, void (^function)(void)) {
  auto popTime = dispatch_time(DISPATCH_TIME_NOW,
                               (int64_t)(delayInSeconds * NSEC_PER_SEC));
  dispatch_after(popTime, dispatch_get_main_queue(), function);
}

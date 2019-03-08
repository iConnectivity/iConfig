/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICViewController.h"

#define UPDATE_INTERVAL 1.0
#define POLLING_PERIOD 0.1
#define NOT_WAITING -1
#define WRITE_WAIT_PERIOD 10
#define UPDATE_PERIOD 10

typedef void (^EventHandler)(UIViewController *viewController);

@interface ICPollingTimer : NSObject

- (id)initWithReadHandler:(EventHandler)readHandler
             writeHandler:(EventHandler)writeHandler;

- (void)writingRequired;
- (void)startPollingTimer:(ICViewController *)sender;
- (void)stopPollingTimer;
- (void)onTimerTick:(NSTimer *)timer;

@end

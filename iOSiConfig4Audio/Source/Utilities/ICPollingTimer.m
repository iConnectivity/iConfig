/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICPollingTimer.h"
#import "ICViewController.h"

@interface ICPollingTimer () {
  NSTimer *pollingTimer;

  EventHandler writeHandler;
  EventHandler readHandler;

  NSInteger writeTime;
  NSInteger readTime;
}

@property(strong) EventHandler writeHandler;
@property(strong) EventHandler readHandler;

@end

@implementation ICPollingTimer

@synthesize writeHandler;
@synthesize readHandler;

- (id)initWithReadHandler:(EventHandler)_readHandler
             writeHandler:(EventHandler)_writeHandler {
  self = [super init];

  if (self != nil) {
    self.readHandler = _readHandler;
    self.writeHandler = _writeHandler;
    writeTime = NOT_WAITING;
    readTime = UPDATE_PERIOD;
  }

  return self;
}

- (void)writingRequired {
  writeTime = WRITE_WAIT_PERIOD;
  readTime = UPDATE_PERIOD;
  [UIApplication sharedApplication].networkActivityIndicatorVisible = YES;
}

- (void)startPollingTimer:(ICViewController *)sender {
  readTime = UPDATE_PERIOD;

  if (pollingTimer != nil) {
    [pollingTimer invalidate];
    pollingTimer = nil;
  }

  pollingTimer = [NSTimer scheduledTimerWithTimeInterval:POLLING_PERIOD
                                                  target:self
                                                selector:@selector(onTimerTick:)
                                                userInfo:sender
                                                 repeats:YES];
}

- (void)stopPollingTimer {
  if (writeTime > 0) {
    if (self.writeHandler != nil) {
      self.writeHandler((ICViewController *)pollingTimer.userInfo);
    }
  }

  [UIApplication sharedApplication].networkActivityIndicatorVisible = NO;

  if (pollingTimer != nil) {
    [pollingTimer invalidate];
    pollingTimer = nil;
  }
}

- (void)onTimerTick:(NSTimer *)timer {
  if ([timer.userInfo isKindOfClass:[ICViewController class]]) {
    if (writeTime > 0) {
      writeTime--;
    } else if (writeTime == 0) {
      writeTime = NOT_WAITING;
      readTime = UPDATE_PERIOD;

      // SEND UPDATE
      if (self.writeHandler != nil) {
        self.writeHandler((ICViewController *)timer.userInfo);
      }

      [UIApplication sharedApplication].networkActivityIndicatorVisible = NO;
    } else if (readTime > 0) {
      readTime--;
    } else if (readTime == 0) {
      readTime = UPDATE_PERIOD;

      if (self.readHandler != nil) {
        self.readHandler((ICViewController *)timer.userInfo);
      }
    }
  }
}

@end

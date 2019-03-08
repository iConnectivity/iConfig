/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <Foundation/Foundation.h>
#import "ICViewController.h"
#import "DeviceID.h"

@interface ICDefaultProvider : NSObject<SquareViewProvider> {
  NSTimer *timer;
}

@property(weak, nonatomic) ICViewController *parent;

+ (NSDictionary *)dictionaryForMenuName:(NSString *)menuName;

- (NSString *)providerName;

- (void)startUpdateTimer;

- (BOOL)stopTimer;

- (void)onTimerHandler;

- (void)onUpdate:(GeneSysLib::DeviceID)deviceID transID:(Word)transID;

- (BOOL)isIndexStandard:(NSUInteger)index;

@end

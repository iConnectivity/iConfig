/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <Foundation/Foundation.h>
#import "ICChoiceDelegate.h"
#import "DeviceInfo.h"

@interface ICAudioMixerChannelV2ChoiceDelegate : NSObject<ICChoiceDelegate>

- (id)initWithDevice:(DeviceInfoPtr)device portID:(Word)portID isInput:(bool)isInput;

- (NSString *)title;
- (NSArray *)options;
- (NSInteger)optionCount;
- (NSInteger)getChoice;
- (void)setChoice:(NSInteger)value;

@end

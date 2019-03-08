/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <Foundation/Foundation.h>
#import "LibTypes.h"
#import "DeviceInfo.h"
#import "ICChoiceDelegate.h"

@interface ICIPModeChoiceDelegate : NSObject<ICChoiceDelegate>
@property(nonatomic, assign) DeviceInfoPtr device;
@property(nonatomic, strong) NSArray *options;

- (id)initWithDevice:(DeviceInfoPtr)device portID:(Word)portID;

- (NSString *)title;
- (NSInteger)optionCount;
- (NSInteger)getChoice;
- (void)setChoice:(NSInteger)value;

@end

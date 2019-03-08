/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <Foundation/Foundation.h>
#import "ICRangeChoiceDelegate.h"
#import "DeviceInfo.h"

@interface ICAudioSyncFactorV2RangeDelegate : NSObject<ICRangeChoiceDelegate>

@property(nonatomic) DeviceInfoPtr device;

+ (ICAudioSyncFactorV2RangeDelegate *)syncFactorWithDevice:
        (DeviceInfoPtr)device;

- (id)initWithDevice:(DeviceInfoPtr)device;

- (NSString *)title;
- (NSInteger)getValue;
- (void)setValue:(NSInteger)value;

- (NSInteger)getMin;
- (NSInteger)getMax;
- (NSInteger)getStride;

@end

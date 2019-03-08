/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <Foundation/Foundation.h>

#import "ICSwitchCellDelegate.h"
#import "DeviceInfo.h"
#import "LibTypes.h"

@interface ICMIDIInputEnabledSwitchDelegate : NSObject<ICSwitchCellDelegate>

@property(nonatomic) DeviceInfoPtr device;
@property(nonatomic) Word portID;

- (id)initWithDevice:(DeviceInfoPtr)device portID:(Word)portID;

- (NSString *)title;
- (BOOL)getValue;
- (void)setValue:(BOOL)value;

@end

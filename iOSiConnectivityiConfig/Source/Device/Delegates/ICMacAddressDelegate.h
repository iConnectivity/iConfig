/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <Foundation/Foundation.h>
#import "ICNormalCellProvider.h"
#import "DeviceInfo.h"
#import "LibTypes.h"

@interface ICMacAddressDelegate : NSObject<NormalCellDelegate> {
  Word portID;
}
@property(nonatomic, assign) DeviceInfoPtr device;

- (id)initWithDevice:(DeviceInfoPtr)device portID:(Word)portID;

- (NSString *)title;
- (NSString *)value;
@end

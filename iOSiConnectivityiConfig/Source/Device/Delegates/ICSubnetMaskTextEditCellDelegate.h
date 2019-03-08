/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <Foundation/Foundation.h>
#import "ICTextEditCellDelegate.h"
#import "DeviceInfo.h"

@interface ICSubnetMaskTextEditCellDelegate : NSObject<ICTextEditCellDelegate>

- (id)initWithDevice:(DeviceInfoPtr)device portID:(Word)portID;

- (NSString *)title;
- (NSString *)getValue;
- (void)setValue:(NSString *)value;

- (NSInteger)maxLength;

- (NSString *)charSet;
- (NSString *)validator;

@end

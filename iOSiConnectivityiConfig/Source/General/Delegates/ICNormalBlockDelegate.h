/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <Foundation/Foundation.h>
#import "LibTypes.h"
#import "DeviceInfo.h"
#import "ICNormalCellProvider.h"

@interface ICNormalBlockDelegate : NSObject<NormalCellDelegate>

@property(strong, nonatomic) NSString *title;
@property(strong, nonatomic) NSString *value;
@property(strong) NormalCellGetBlock getValue;

- initWithTitle:(NSString *)title value:(NSString *)value;
- initWithTitle:(NSString *)title getValue:(NormalCellGetBlock)getValue;

@end

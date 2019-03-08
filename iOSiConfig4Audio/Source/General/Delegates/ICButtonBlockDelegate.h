/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <Foundation/Foundation.h>
#import "LibTypes.h"
#import "DeviceInfo.h"
#import "ICButtonCellProvider.h"

@interface ICButtonBlockDelegate : NSObject<ButtonCellDelegate>

@property(strong, nonatomic) NSString *title;
@property(strong, nonatomic) NSString *value;
@property(strong) ButtonCellFunc func;

- initWithTitle:(NSString *)title value:(NSString *)value func:(ButtonCellFunc)func;
- (void) doFunc;

@end

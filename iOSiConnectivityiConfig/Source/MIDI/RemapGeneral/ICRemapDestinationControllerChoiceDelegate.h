/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICControllerChoiceDelegate.h"
#import "DeviceInfo.h"

@interface ICRemapDestinationControllerChoiceDelegate
    : ICControllerChoiceDelegate

- (id)initWithDevice:(DeviceInfoPtr)device
              portID:(Word)portID
           remapType:(GeneSysLib::RemapTypeEnum)remapType
             remapID:(int)remapID;

- (NSString *)title;

- (NSInteger)getChoice;

- (void)setChoice:(NSInteger)value;

@end

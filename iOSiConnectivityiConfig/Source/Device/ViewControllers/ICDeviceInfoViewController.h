/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>
#import "DeviceInfo.h"
#import "ICBaseInfoViewController.h"
#import "Communicator.h"
#import "Info.h"
#import "InfoList.h"

@interface ICDeviceInfoViewController : ICBaseInfoViewController {
}

- (id)initWithCommunicator:(GeneSysLib::CommPtr)comm
                    device:(DeviceInfoPtr)device;

@end

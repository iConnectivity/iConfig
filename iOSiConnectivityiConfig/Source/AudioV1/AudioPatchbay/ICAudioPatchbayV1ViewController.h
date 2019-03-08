/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>
#import "Communicator.h"
#import "DeviceInfo.h"

@interface ICAudioPatchbayV1ViewController : UITableViewController {
  GeneSysLib::CommPtr comm;
  DeviceInfoPtr device;
}

@property(nonatomic, strong) IBOutlet UIView *headerView;

- (id)initWithCommunicator:(GeneSysLib::CommPtr)comm
                    device:(DeviceInfoPtr)device;

@end

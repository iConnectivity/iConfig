/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>
#import "Communicator.h"
#import "DeviceInfo.h"

@interface ICAudioPortPatchbayV1ViewController
    : UIViewController<UITableViewDataSource, UITableViewDelegate> {
  GeneSysLib::CommPtr comm;
  DeviceInfoPtr device;
  GeneSysLib::FlatAudioPortPatchbay selectedFlatPortPatchbay;
}

@property(nonatomic, strong) IBOutlet UITableView *tableView;
@property(nonatomic, strong) IBOutlet UILabel *destLabel;

- (id)initWithCommunicator:(GeneSysLib::CommPtr)comm
                    device:(DeviceInfoPtr)device
              flatPatchbay:(GeneSysLib::FlatAudioPortPatchbay)flatPatchbay;
@end

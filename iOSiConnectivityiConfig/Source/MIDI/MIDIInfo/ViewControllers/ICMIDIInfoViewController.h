/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>

#import "Communicator.h"
#import "DeviceInfo.h"
#import "ICBaseInfoViewController.h"
#import "ICMIDIIndexTableViewController.h"
#import "MIDIInfo.h"

@interface ICMIDIInfoViewController
    : ICBaseInfoViewController<SidebarDelegate> {
}

- (id)initWithMIDIInfo:(GeneSysLib::MIDIInfo)midiInfo
          communicator:(GeneSysLib::CommPtr)comm
                device:(DeviceInfoPtr)device;

- (void)setupRightMenuButton;
- (void)rightDrawerButtonPress:(id)sender;

- (void)indexPathSelected:(NSIndexPath *)indexPath;

@end

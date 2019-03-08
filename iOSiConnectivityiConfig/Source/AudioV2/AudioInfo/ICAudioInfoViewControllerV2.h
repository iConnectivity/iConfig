/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>
#import "ICBaseInfoViewController.h"
#include "Communicator.h"
#include "DeviceInfo.h"

@interface ICAudioInfoViewControllerV2
    : ICBaseInfoViewController<UIAlertViewDelegate> {
  UIBarButtonItem *mainMenuButton;
  UIBarButtonItem *commitButton;
  UIAlertView *confirmAlert;
  UIAlertView *commitConfirmAlert;
  UIAlertView *commitCompleteAlert;
  UIAlertView *commitFailedAlert;
  BOOL change;

  GeneSysLib::CommPtr comm;
  DeviceInfoPtr device;
}

- (id)initWithCommunicator:(GeneSysLib::CommPtr)comm
                    device:(DeviceInfoPtr)device;

- (void)onMainMenuPressed;
- (void)onCommitPressed;

- (void)alertView:(UIAlertView *)alertView
    clickedButtonAtIndex:(NSInteger)buttonIndex;

@end
